To compile the client on linux, type make into the command line in the appropriate directory. The run it type ./tcp_client (address of server) > (optional output for stdout)
Example: ./tcp_client http://www.google.com > output.html

Obstacles: 
The biggest technical obstacle was figuring out exactly what needed to be put into the various connect and receive methods to ensure that a connection could be established and would not simply sit forever trying to connect to a server that doesn't exist.
There was also a connection refused error that I kept running into. This was because I was trying to connect using an alias instead of an IP address.
Lastly when using gethostbyname if there are multiple IP's associated with a particular server, sometimes the first one leads to a 403 error. Adding the www. to the beginning of the address seems to solve this problem.
