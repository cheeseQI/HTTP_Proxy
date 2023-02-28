### Danger log of Project2 (HTTP in Proxy)
### NetID: sq44, yw540

#### 1.Terminate when throws exception without any backups

If encountering an error type, the program will throw an exception and terminate without implementing any restarting service or high availability measures.
  

#### 2.Physical problem of Cache Structure
We find that out caching mechanism has a fixed capacity and lacks elasticity. In order to fix it, we can consider methods including increasing cache capacity, implementing cache eviction strategy, adding cache warming function, introducing distributed cache, and implementing cache automatic scaling, which need a lot of time to plan.


#### 3.No test of the high concurrency
We find that our high concurrency system needs to be tested and there is a possibility of crashes. We can conduct load testing, implement proper error handling as well as more exception logging. In addition, distributed systems is better but it's hard for us currently.


#### 4.Lack of security
For a high-traffic proxy, the lack of security protection such as protection against attacks is a concern. That is the basic thought of a software.
