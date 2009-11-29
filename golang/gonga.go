// Original Ngaro, RetroForth, Uki, ...
//	Copyright (C) 2008, 2009, Charles Childers
// Go port
//	Copyright 2009 JGL
// Public Domain or LICENSE file

/*
	Gonga launchs a new ngaro image (retroForth by default) and
	launchs an Ngaro VM. Its I/O ports are connected to goroutines
	that read from stdin and write to stdout.

	From inside, you can launch new children VMs, or save the
	current image to a dump file: the parent image is saved to
	the file specified with the flag -d, child images are saved to
	files finishing in .n (where n is a number).

	When launching gonga it is posible to specify the image to
	load. Multiple images can be given (they will be concatenated)
	the special filename "-" means to load the default image.

	Usage:
		gonga [options] [image file(s)]
	Options:
		-c=16: number of children per VM
		-h=false: displays help and exit
		-s=50000: image size
		-d="": image dump file
*/

package main

import (
	"./ngaro";
	"bufio";
	"flag";
	"fmt";
	"io";
	"os";
)

var children = flag.Int("c", 16, "number of children per VM")
var size = flag.Int("s", 50000, "image size")
var dump = flag.String("d", "retro.img", "image dump file")
var help = flag.Bool("h", false, "displays help and exit")
var verbose = flag.Bool("v", false, "verbose output")

func perror(s ...) {
	if *verbose {
		fmt.Fprint(os.Stderr, s)
	}
}

func readIn(vm *ngaro.NgaroVM) {
	r := bufio.NewReader(os.Stdin);
	b := make([]byte, 1);
	for {
		_, err := io.ReadAtLeast(r, b, 1);
		if err != nil {
			vm.Off <- true
		}
		vm.Write(b);
	}
}

func printOut(vm *ngaro.NgaroVM) {
	b := make([]byte, 1);
	for {
		_, err := vm.Read(b);
		if err != nil {
			vm.Off <- true
		}
		fmt.Print(string(b))
	}
}

func main() {
	flag.Parse();
	if *help {
		flag.Usage();
		os.Exit(0);
	}
	ngaro.Verbose = *verbose;
	ngaro.ClearScreen = func () {
		fmt.Printf("\033[2J\033[1;1H")	// Clear screen
	};

	var img = make([]int, *size);
	perror("Gonga: loading ");
	if flag.NArg() == 0 {
		img = &retroImage;
		perror("default (retroForth-10.3) ");
	}
	for i, n := 0, 0; i < flag.NArg(); i++ {
		if flag.Arg(i) == "-" {
			perror("default (retroForth-10.3) ");
			img = &retroImage;
			img = img[0:cap(img)];
			n += len(retroImage);
		} else {
			perror(flag.Arg(i), " ");
			n += ngaro.LoadDump(flag.Arg(i), img, n);
		}
	}
	vm := ngaro.NewVM(img, *size, 16, *dump);
	go readIn(vm);
	go printOut(vm);
	<-vm.Off;
}
