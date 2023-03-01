We have provided normal test cases, and also high occurance test cases in normal/swarm mode.

## 1. HTTP Test Cases
### get
functional test:
http://httpbin.org/
chunked test:
http://www.httpwatch.com/httpgallery/chunked/chunkedimage.aspx
test for delay:
http://httpbin.org/delay/5
### post
http://httpbin.org/forms/post


## 2. HTTPS Test Cases
### connect
go any where of google! You can google cat, dog, or anything!
if you want to watch video, go to youtube, eg:
https://www.youtube.com/

## 3.Concurrency(Pressure) Test Cases
after you have checked the docker version, you can run this command and build docker swarm service:
`sudo docker stack deploy -c docker-compose.yml --with-registry-auth my-app`
then you can re-scale the service to multiple replicates:
`docker service scale my-app_proxy=5`
### Apache Bench test with 1000 request
note: you should change the url to your local host
ab -n 1000 -c 100 -X vcm-30653.vm.duke.edu:12345 http://example.com/