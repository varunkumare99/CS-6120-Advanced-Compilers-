## week2
**Programs are represented in an Intermediate Language called** [Bril](https://capra.cs.cornell.edu/bril/lang/index.html)
- Goal: Given a program in Bril convert it into a Control Flow Graph(CFG)

- For each function present in the Bril program a correponding CFG is created.
- The output is a svg file. (output file naming: ```inputFileName_{function number}.svg```
- To convert the Bril program to json use [bril2json](https://capra.cs.cornell.edu/bril/tools/text.html)
- Testing is done using [Turnt](https://github.com/cucapra/turnt)

Dependencies
- [jsoncpp](https://github.com/open-source-parsers/jsoncpp) 
- [boost graph library](https://www.boost.org/doc/libs/1_80_0/libs/graph/doc/)

**Solution**
```	
Parse Json Input file (Bril program)
	For each function in program
		create basicblock
		if instruction is a terminator(br/jmp/ret)
			append instruction to basicblock
		else
			create new basicblock

//each basic block is vertex in our CFG
// there is an edge from BasicBlock 'A' to BasicBlock 'B' if the program flows from 'A' to 'B'

	For each basicBlock in basicBlocksList
		if instr is jmp
			add edge from current basicblock to basickblock in jmp label
		else instr is br
			add edge from current basicblock to basicblock in br label(true)
			add edge from current basicblock to basicblock in br label(false)
		else if not return instruction and next block exits
			add edge from current basicblock to next basicblock
		
```
#### build
```
$ cd Projects/week2
$ mkdir build
$ cd build
$ cmake -GNinja ..
$ ninja
$ ./createCFG <bril program in json>
```
```
example
$ ./createCFG ../testFiles/eight-queens.json
Following CFG's are created with fileNames
- eight-queens_0.svg
- eight-queens_1.svg
- eight-queens_2.svg
```

#### testing
```
$ cd test
turnt eight-queens.json
turnt simpleEx.json
```
#### Result: Control Flow Graph
![output0](https://github.com/varunkumare99/CS-6120-Advanced-Compilers-/blob/main/Projects/week2/CFG_img/eight-queens0.svg)

![output1](https://github.com/varunkumare99/CS-6120-Advanced-Compilers-/blob/main/Projects/week2/CFG_img/eight-queens1.svg)
![output2](https://github.com/varunkumare99/CS-6120-Advanced-Compilers-/blob/main/Projects/week2/CFG_img/eight-queens2.svg)
