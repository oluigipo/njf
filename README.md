# NJF: Not a Json File
This is a project I wanted to make, and so I did.
### What is NJF?
It´s simple. A NJF file is made of 3 things: Objects, Flags, and Bodies. The file itself is just a big Body.
An Object is a thing with a name, which can also have any number of Flags and one Body. Here is an example:
```
"Object Name" flag1 flag2 512 {
	"An Object inside a Body!" -100
	"\"Another\" Object" {
		"Don't read:" Never gonna give you up :)
	}
}
```
As you can see, flags can also be a number!
### Can I use NJF in a project?
Please, no...
This was not made to be taken seriously.
### What are those files in this repo?
It´s a single header library made in C for this project. I wrote it (at least I tried) to be simple, so feel free to change anything you want.
To use it, you just need to define `NJF_IMPLEMENTATION` before including `njf.h` in **a single translation unit**.
You can see some (or just one) examples in the `examples/` folder.