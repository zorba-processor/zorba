#Zorba - NoSQL Query Processor
[![Circle CI](https://circleci.com/gh/28msec/zorba.svg?style=svg)](https://circleci.com/gh/28msec/zorba) 

http://zorba.io/

##Docker Container
A container with the zorba binary is available at https://registry.hub.docker.com/u/fcavalieri/zorba/

Below are examples of Zorba executed via Docker:

```bash
docker run zorba -q 1+1
```

You can also mount a local folder that contains your queries. The image exposes a volume at `/queries`:

```bash
docker run -v /home/fcavalieri/myqueries:/queries zorba -q foo.xq -f
```

##Documentation
http://zorba.io/documentation/latest

##Support
If you want to be informed about new code releases, bug fixes and general news and information about the Zorba XQuery Processor, subscribe to the Zorba Users mailing list [zorba-users@googlegroups.com](mailto:zorba-io-users@googlegroups.com)

