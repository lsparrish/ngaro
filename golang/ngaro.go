// Original Ngaro Virtual Machine and Uki framework:
//	Copyright (C) 2008, 2009, Charles Childers
// Go port
//	Copyright 2009 JGL
// Public Domain or LICENSE file

// TODO:
//	- writeImage()
//	- Add more i/o ports
//	- Better error handling

/*
	Ngaro virtual machines.

	Ngaro is a portable virtual machine / emulator for a dual
	stack processor and various I/O devices. The instruction set
	is concise (31 core instructions), and the basic I/O devices
	are kept minimal. For more information see
		http://github.com/crcx/ngaro

	Communication with the virtual machine is done through
	int chanels for input (port 1) and output (port 2).

	In addition to normal ngaro features, this Go version
	allows to launch 4 new children virtual machines (which
	can launch 4 more children, and so on) through the port 6.
	The port 7 can be used to create pipes between the
	input and output of two childrens.

	: reset ( n- ) 1 6 out wait ;
	: pipe ( xy- ) 1 7 out wait ;
	: child.in ( xy- ) 8 + 1 swap out wait ; ( for children = 4 )
	: child.out ( x-y ) 12 + 1 swap out wait ; ( for children = 4 )
*/

package ngaro

import (
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
)

type NgaroVM struct {
	size		int;
	img		[]int;
	dump		string;
	tos, nos	int;
	ports		[]int;
	children	int;
	child		[]*NgaroVM;
	In, Out		chan int;
	Off		chan bool;
}

func (vm *NgaroVM) writeDump() {
	println("saving to :", vm.dump);
	w, err := os.Open(vm.dump, os.O_WRONLY|os.O_CREATE, 0666);
	if err != nil {
		return;
	}
	// this write always fails (with constants as 3rd arg too)
	err = B.Write(w, B.BigEndian, vm.img[0:]);
	if err != nil {
		println("ERROR writing image");
	}
}

func (vm *NgaroVM) wait() (spdec int) {
	//D:print("\tWAITING");
	switch 1 {
	case vm.ports[0]:
		return

	case vm.ports[1]:	// Input (Port 1)
		vm.ports[1] = <-vm.In;
		vm.ports[0] = 1;

	case vm.ports[2]:	// Output (Port 2)
		vm.Out <- vm.tos;
		vm.ports[2] = 0;
		vm.ports[0] = 1;
		spdec = 1;

	case vm.ports[4]:	// Save Image (Port 4)
		vm.writeDump();
		vm.ports[4] = 0;
		vm.ports[0] = 1;

	case vm.ports[13]:	// Reset Child (Port 13)
		c := vm.tos;
		if c < 0 || c > len(vm.child) {
			return
		}
		if vm.child[c] == nil {
			vm.child[c] = NewVM(vm.img, vm.size, vm.children, "")
		} else {
			vm.child[c].Off <- false
		}
		vm.ports[0] = 1;
		spdec = 1;

	case vm.ports[14]:	// Pipe (Port 14)
		f := vm.tos;
		t := vm.nos;
		if f < 0 || f > len(vm.child) || t < 0 || t > len(vm.child) {
			return
		}
		go func() {
			for {
				vm.child[t].Out <- <-vm.child[f].In
			}
		}();
		vm.ports[0] = 1;
		spdec = 2;
	}

	for i, c := range vm.child {
		if c != nil {	// Children VMs I/O (Ports 16-...)
			if vm.ports[16+i] == 1 {
				c.In <- vm.tos;
				vm.ports[16+i] = 0;
				vm.ports[0] = 1;
				spdec = 1;
				return;
			}
			if vm.ports[16+vm.children+i] == 1 {
				vm.ports[16+vm.children+i] = <-c.Out;
				vm.ports[0] = 1;
				return;
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
	//D:println("Ngaro: new core");
	var x, y int;
	var sp, rsp int;
	var data, addr [stackDepth]int;
	vm.img = make([]int, vm.size);
	for i, x := range image {
		vm.img[i] = x
	}
	for ip := 0; ip < vm.size; ip++ {
		//D:print("NgaroVM (", vm, "): Op[ ip =", ip, "]\t= ", vm.img[ip], "\t-->");
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
			if data[sp] < 0 || data[sp] > len(vm.ports) {
				println(" ERROR (Invalid port)");
				break;
			}
			x = data[sp];
			data[sp] = vm.ports[x];
			vm.ports[x] = 0;
		case Out:
			if data[sp] < 0 || data[sp] > len(vm.ports) {
				println(" ERROR: Invalid port");
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
			println(" ERROR: Stack underflow");
			sp = 0;
		} else if stackDepth-sp < 2 {
			println(" ERROR: Stack overflowing (2 elements droped)");
			sp -= 2;
		}
		vm.tos = data[sp];
		if sp > 1 {
			vm.nos = data[sp-1]
		} else {
			vm.nos = 0
		}
		//D:println("\tsp =", sp, "\trsp =", rsp, "\ttos =", vm.tos);
	}
	vm.Off <- true;
}

func NewVM(image []int, size, children int, dump string) *NgaroVM {
	println("Ngaro: New core ( size:", size, ")");
	if len(image) > size {
		println("Ngaro: image too large");
		return nil;
	}
	vm := new(NgaroVM);
	vm.size = size;
	vm.children = children;
	vm.child = make([]*NgaroVM, children);
	vm.ports = make([]int, 16+2*children);
	vm.dump = dump;
	vm.In = make(chan int);
	vm.Out = make(chan int);
	vm.Off = make(chan bool);
	go vm.core(image);
	return vm;
}
