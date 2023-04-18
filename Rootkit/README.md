<h1>
  Linux/Unix Rootkit
</h1>

<h3>
  src directory
</h3>

<hr>

<h3>
  installation of rootkit with seperate files:
  <h4>
    <ol type="1">
       <li>make sure "make" is installed</li>
       <li>move the src/custom  folder to /lib/modules/$(uname -r)/build</li>
       <li>create a directory with the build/Makefile and the src/rootkit.c</li>
       <li>run "make"</li>
    </ol>
  </h4>
</h3>
