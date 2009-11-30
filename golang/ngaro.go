// Original Ngaro Virtual Machine and Uki framework:
//	Copyright (C) 2008, 2009, Charles Childers
// Go port
//	Copyright 2009 JGL
// Public Domain or LICENSE file

/*
	Ngaro virtual machines.

	Ngaro is a portable virtual machine / emulator for a dual
	stack processor and various I/O devices. The instruction set
	is concise (31 core instructions), and the basic I/O devices
	are kept minimal. For more information see
		http://github.com/crcx/ngaro

	Communication with the virtual machine is done through
	int chanels for input (port 1) and output (port 2). The
	port 4 can be used to save the current image to a file,
	while port 5 is used to get information about the virtual
	machine.

	In addition to normal ngaro features, this Go version
	allows to launch new children virtual machines (which
	can launch more children, and so on) through the port 13.

	Some useful words:

	: reset ( n- ) 1 13 out wait ;
	: children ( -n ) -7 5 out wait 5 in ;
	: c<- ( xy- ) 16 + 1 swap out wait ;
	: <-c ( x-y ) 16 + children + dup 1 swap out wait in ;
*/

package ngaro

import (
	"fmt";
	"os";
	B "encoding/binary";
)

const (
	// Instruction set
	Nop=iota; Lit; Dup; Drop; Swap; Push; Pop;
	Call; Jump; Return; GtJump; LtJump; NeJump;
	EqJump; Fetch; Store; Add; Sub; Mul; Dinod;
	And; Or; Xor; ShL; ShR; ZeroExit; Inc; Dec;
	In; Out; Wait;

	stackDepth	= 100;
	chanBuffer	= 128;
)

type NgaroVM struct {
	size		int;
	img		[]int;
	dump		string;
	tos, nos	int;
	ports		[]int;
	children	int;
	child		[]*NgaroVM;
	in, out		chan int;
	Off		chan bool;
}

var Verbose bool = false

var ClearScreen func() = func() {}

func perror(s ...) {
	if Verbose {
		fmt.Fprint(os.Stderr, s)
	}
}

func (vm *NgaroVM) Read(p []byte) (n int, err os.Error) {
	for i, _ := range p {
		if x := <-vm.out; x < 0 {
			ClearScreen()
		} else {
			p[i] = byte(x);
			n++;
		}
	}
	return;
}

func (vm *NgaroVM) Write(p []byte) (n int, err os.Error) {
	for _, b := range p {
		vm.in <- int(b);
		n++;
	}
	return;
}

func LoadDump(filename string, img []int, start int) int {
	r, err := os.Open(filename, os.O_RDONLY, 0);
	if err != nil {
		return 0
	}
	var ui uint32;
	var i int;
	for i, _ = range img[start:] {
		if err := B.Read(r, B.LittleEndian, &ui); err != nil {
			break
		}
		img[i] = int(ui);
	}
	perror(" [ Ngaro: loaded image ", filename, " ] ");
	return i;
}

func (vm *NgaroVM) WriteDump(filename string) {
	w, err := os.Open(filename, os.O_WRONLY|os.O_CREATE, 0666);
	if err != nil {
		return
	}
	for _, i := range vm.img {
		if err = B.Write(w, B.LittleEndian, uint32(i)); err != nil {
			perror(" [ Ngaro ERROR: writing ", filename, "] ")
		}
	}
	perror(" [ Ngaro: saved image to ", filename, " ( size:", vm.size, " ) ] ");
}

func (vm *NgaroVM) wait() (spdec int) {
	switch 1 {
	case vm.ports[0]:
		return

	case vm.ports[1]:	// Input (Port 1)
		vm.ports[1] = <-vm.in;
		vm.ports[0] = 1;

	case vm.ports[2]:	// Output (Port 2)
		vm.out <- vm.tos;
		vm.ports[2] = 0;
		vm.ports[0] = 1;
		spdec = 1;

	case vm.ports[4]:	// Save Image (Port 4)
		vm.WriteDump(vm.dump);
		vm.ports[4] = 0;
		vm.ports[0] = 1;

	case vm.ports[13]:	// Reset Child (Port 13)
		vm.ports[13] = 0;
		vm.ports[0] = 1;
		spdec = 1;
		c := vm.tos;
		if c < 0 || c > len(vm.child)-1 {
			return
		}
		if vm.child[c] == nil {
			d := vm.dump;
			if d != "" {
				d += "." + string(vm.tos)
			}
			vm.child[c] = NewVM(vm.img, vm.size, vm.children, d);
		} else {
			vm.child[c].Off <- false
		}
	}

	for i, c := range vm.child {
		if c != nil {	// Children VMs I/O (Ports 16-...)
			if vm.ports[16+i] == 1 {
				c.in <- vm.tos;
				vm.ports[16+i] = 0;
				vm.ports[0] = 1;
				spdec = 1;
			}
			if vm.ports[16+vm.children+i] == 1 {
				vm.ports[16+vm.children+i] = <-c.out;
				vm.ports[0] = 1;
			}
		}
	}

	switch vm.ports[5] {	// Capabilities (Port 5)
	case 0:
		return

	case -1:	// Image size
		vm.ports[5] = vm.size;
		vm.ports[0] = 1;

	case -5:	// Stack depth
		vm.ports[5] = stackDepth;
		vm.ports[0] = 1;

	case -6:	// Address stack depth
		vm.ports[5] = stackDepth;
		vm.ports[0] = 1;

	case -7:	// Number of children
		vm.ports[5] = vm.children;
		vm.ports[0] = 1;

	default:
		vm.ports[5] = 0;
		vm.ports[0] = 1;
	}
	return;
}

func (vm *NgaroVM) core(image []int) {
	var x, y int;
	var sp, rsp int;
	var data, addr [stackDepth]int;
	vm.img = make([]int, vm.size);
	for i, x := range image {
		vm.img[i] = x
	}
	for ip := 0; ip < vm.size; ip++ {
		if end, sig := <-vm.Off; sig {
			if end {
				break
			}
			ip = 0;
		}
		switch vm.img[ip] {
		case Nop:
		case Lit:
			sp++;
			ip++;
			data[sp] = vm.img[ip];
		case Dup:
			sp++;
			data[sp] = data[sp-1];
		case Drop:
			data[sp] = 0;
			sp--;
		case Swap:
			data[sp], data[sp-1] = data[sp-1], data[sp]
		case Push:
			rsp++;
			addr[rsp] = data[sp];
			sp--;
		case Pop:
			sp++;
			data[sp] = addr[rsp];
			rsp--;
		case Call:
			ip++;
			rsp++;
			addr[rsp] = ip;
			ip = vm.img[ip] - 1;
		case Jump:
			ip++;
			ip = vm.img[ip] - 1;
		case Return:
			ip = addr[rsp];
			rsp--;
		case GtJump:
			ip++;
			if data[sp-1] > data[sp] {
				ip = vm.img[ip] - 1
			}
			sp = sp - 2;
		case LtJump:
			ip++;
			if data[sp-1] < data[sp] {
				ip = vm.img[ip] - 1
			}
			sp = sp - 2;
		case NeJump:
			ip++;
			if data[sp-1] != data[sp] {
				ip = vm.img[ip] - 1
			}
			sp = sp - 2;
		case EqJump:
			ip++;
			if data[sp-1] == data[sp] {
				ip = vm.img[ip] - 1
			}
			sp = sp - 2;
		case Fetch:
			data[sp] = vm.img[data[sp]]
		case Store:
			vm.img[data[sp]] = data[sp-1];
			sp = sp - 2;
		case Add:
			data[sp-1] += data[sp];
			data[sp] = 0;
			sp--;
		case Sub:
			data[sp-1] -= data[sp];
			data[sp] = 0;
			sp--;
		case Mul:
			data[sp-1] *= data[sp];
			data[sp] = 0;
			sp--;
		case Dinod:
			x = data[sp];
			y = data[sp-1];
			data[sp] = y / x;
			data[sp-1] = y % x;
		case And:
			x = data[sp];
			y = data[sp-1];
			sp--;
			data[sp] = x & y;
		case Or:
			x = data[sp];
			y = data[sp-1];
			sp--;
			data[sp] = x | y;
		case Xor:
			x = data[sp];
			y = data[sp-1];
			sp--;
			data[sp] = x ^ y;
		case ShL:
			x = data[sp];
			y = data[sp-1];
			sp--;
			data[sp] = y << uint(x);
		case ShR:
			x = data[sp];
			y = data[sp-1];
			sp--;
			data[sp] = y >> uint(x);
		case ZeroExit:
			if data[sp] == 0 {
				sp--;
				ip = addr[rsp];
				rsp--;
			}
		case Inc:
			data[sp]++
		case Dec:
			data[sp]--
		case In:
			if data[sp] < 0 || data[sp] > len(vm.ports)-1 {
				perror(" [ Ngaro ERROR: Invalid port ] ");
				break;
			}
			x = data[sp];
			data[sp] = vm.ports[x];
			vm.ports[x] = 0;
		case Out:
			if data[sp] < 0 || data[sp] > len(vm.ports) {
				perror(" [ Ngaro ERROR: Invalid port ] ");
				break;
			}
			vm.ports[0] = 0;
			vm.ports[data[sp]] = data[sp-1];
			sp = sp - 2;
		case Wait:
			sp -= vm.wait()
		default:
			ip = vm.size
		}
		// to avoid segfaults:
		if sp < 0 {
			perror(" [ Ngaro ERROR: Stack underflow ] ");
			sp = 0;
		} else if stackDepth-sp < 2 {
			perror(" [ Ngaro ERROR: Stack overflow (2 elements droped) ] ");
			sp -= 2;
		}
		vm.tos = data[sp];
		if sp > 1 {
			vm.nos = data[sp-1]
		} else {
			vm.nos = 0
		}
	}
	vm.Off <- true;
}

func NewVM(image []int, size, children int, dump string) *NgaroVM {
	if len(image) > size {
		perror(" [ Ngaro ERROR: image too large ] ");
		return nil;
	}
	vm := new(NgaroVM);
	vm.size = size;
	vm.children = children;
	vm.child = make([]*NgaroVM, children);
	vm.ports = make([]int, 16+2*children);
	vm.dump = dump;
	vm.in = make(chan int, chanBuffer);
	vm.out = make(chan int, chanBuffer);
	vm.Off = make(chan bool);
	go vm.core(image);
	perror(" [ Ngaro: new core ( size:", size, ") ] ");
	return vm;
}
