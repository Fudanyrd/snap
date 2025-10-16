# Coding

## Optimize Centrality Computation

The [centrality program](./examples/centrality/centrality.cpp) has not
been optimized yet. We expect it to run at least 1.5x faster, while still produce
reasonable output.

Advice on how to do this:

<ul>
  <li> An example of concurrent BFS implementation is in 
    <a href="examples/bfs-demo/bfs.cpp">examples/bfs-demo</a> 
    Make sure you understand this code before contributing, especially 
    the API of <a href="glib-core/thread.cpp">thread pool</a>.
  </li>
  <li> Focus on parts with high-overhead. A list of high-overhead functions is shown 
    <a href="./report/images/centr.base.txt"> here </a>. 
  </li>

  <li> 
    Use the thread pool to execute these functions concurrently. 
  </li>

  <li> 
    Use more efficient data structures. Always consider <a href="https://gcc.gnu.org/onlinedocs/gcc/_005f_005fatomic-Builtins.html">
    atomic builtins 
    </a> to avoid locks.
  </li>
</ul>

# Report

These parts of our report are missing or incomplete:

<ul>
  <li> Introduction </li>
  <li> Conclusion </li>
  <li> Evaluation~Centrality </li>
  <li> Limitation and Future Work </li>
</ul>

