# Contributing to This Repository

See [TODO.md](./TODO.md) for what you can contribute!

## Building Snap Library

Linux Dependencies:

<ul>
  <li>C++ compiler with c++98 support. </li>
  <li>bash and python3 </li>
  <li>GNU Make</li>
  <li>Linux perf (for development)</li>
</ul>

After installing these dependencies, simply run `make all`.

## Building Report

Our project report is in the [report](./report) folder. To build the .pdf report, you have to:

Dependencies:

<ul>
  <li>python3~matplotlib</li>
  <li>pdflatex </li>
</ul>

<ol>
  <li> Download the ACM latex template; </li>
  <li> The "report/images" folder contains a <a href="./report/images/Makefile">Makefile</a>, you have to build
 images with it; </li>
  <li> Now you can trigger <i>pdflatex</i> on <a href="report/main.tex">main.tex</a>. </li>
</ol>

> Some images of visualization result  is in [report/viz](./report/viz) folder,
> which are NOT tracked by Git.

## Adding C++ Source Files

If you wish to add C++ source files to the `snap-core`, `glib-core`  or `examples`
folder,  you may want to look at or edit these configure scripts:
<ul>
  <li> <a href="snap-core/configure">snap-core/configure</a> </li>
  <li> <a href="glib-core/configure">glib-core/configure</a> </li>
  <li> <a href="examples/configure">examples/configure</a> </li>
</ul>


## Contributing to Project Technique Report

Please follow these rules when contributing to project report:

<ul>
  <li> Add image data and build scripts to the <i>report/images</i> folder if possible;
      avoid tracking binary image files with git. </li>
  <li> Instead of directly coding latex table, you are encouraged to add table data to <i>report/images</i>,
  and use the <a href="report/images/tablegen.py">tablegen</a> script to automatically generate tables. 
  <li> Before committing, use a spell checker such as aspell to check your modified latex files. </li>
</ul>

