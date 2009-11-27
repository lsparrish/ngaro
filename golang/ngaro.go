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
*/

package ngaro

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
	size	int;
	tos	int;
	ports	[16]int;
	In, Out	chan int;
	Off	chan bool;
}

func (vm *NgaroVM) writeImage() {
	// Write to what?
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
		go vm.writeImage();
		vm.ports[4] = 0;
		vm.ports[0] = 1;
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
	img := make([]int, vm.size);
	for i, x := range image {
		img[i] = x
	}
	for ip := 0; ip < vm.size; ip++ {
		//D:print("NgaroVM: Op[ ip =", ip, "]\t= ", img[ip], "\t-->");
		switch img[ip] {
		case Nop:
		case Lit:
			sp++;
			ip++;
			data[sp] = img[ip];
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
			ip = img[ip] - 1;
		case Jump:
			ip++;
			ip = img[ip] - 1;
		case Return:
			ip = addr[rsp];
			rsp--;
		case GtJump:
			ip++;
			if data[sp-1] > data[sp] {
				ip = img[ip] - 1
			}
			sp = sp - 2;
		case LtJump:
			ip++;
			if data[sp-1] < data[sp] {
				ip = img[ip] - 1
			}
			sp = sp - 2;
		case NeJump:
			ip++;
			if data[sp-1] != data[sp] {
				ip = img[ip] - 1
			}
			sp = sp - 2;
		case EqJump:
			ip++;
			if data[sp-1] == data[sp] {
				ip = img[ip] - 1
			}
			sp = sp - 2;
		case Fetch:
			data[sp] = img[data[sp]]
		case Store:
			img[data[sp]] = data[sp-1];
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
		} else if sp > stackDepth {
			println(" ERROR: Stack overflow (cleared)");
			sp = 0;
		}
		vm.tos = data[sp];
		//D:println("\tsp =", sp, "\trsp =", rsp, "\ttos =", vm.tos);
	}
	vm.Off <- true;
}

func NewVM(image []int, size int) *NgaroVM {
	//D:println("Ngaro: New core (", len(image), "/", size, ")");
	if len(image) > size {
		println("Ngaro: image too large");
		return nil;
	}
	vm := new(NgaroVM);
	vm.size = size;
	vm.In = make(chan int);
	vm.Out = make(chan int);
	vm.Off = make(chan bool);
	go vm.core(image);
	return vm;
}
