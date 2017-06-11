This paper presents our strategic solution in order to optimize the concurrent queries on a large-scale directed, dynamic graph such as a social network. For this kind of big graphs, edges can be dynamically added or removed while a user asks to determine the shortest path between two vertices. Based on the ideas of the previous working groups in the SigMod Programming Contest 2016, we propose a strategic solution in order to solve this challenge. This strategic solution is based on (i) an appropriate data structure, (ii) the optimized update actions (insertions and deletions) and (iii) by improving the performance of query processing by both reducing the searching space and computing in multithreaded parallel. 
Thus, graph is globally organized by the adjacent lists in order to improve the cache hit ratio and the update action performance. The reduction of searching space is based on the way of calculating the potential enqueued vertices. Cilkplus method is chosen to takes a lot of advantages of the capabilities of modern computer architectures such as multi-threads, multi-cores. It allows to parallelize the consecutive queries efficiently. Our experimental results are also outstanding compared to others existing solutions by using popular datasets such SigMod Contest 2016 and SNAP DataSet Collections.

This project is built for the paper "Towards a Strategic Solution for Optimizing the Concurrent Queries on Large-Scale Dynamic Graph".

To get the datasets,please surf at the following pages:

    For Pokec social network: https://snap.stanford.edu/data/soc-pokec.html
    For LiveJournal social network: https://snap.stanford.edu/data/soc-LiveJournal1.html 
    
The SigMod dataset was uploaded in the "corpus" folder.
