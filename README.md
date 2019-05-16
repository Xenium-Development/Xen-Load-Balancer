# Xen-Load-Balancer
An open-source cross platform load balancer for Windows and Linux (Ubuntu Tested).

This is an application I developed for my final year report in Birmingham City University.

While my project was designed mainly for Raspberry pi's, Circumstances changed and thus a cross-platform approach was used instead.

It is an application-based load balancer using the Round-Robin approach.

It is compatible with the following:
* Windows (Visual Studio 2017)
* Ubuntu (g++)

*Will work with previous versions of VS and more than likely work with other distros of Linux*

There are two configuration files included inside the folder, these are to be used with the compiled executable.

## config.json
This is not required and the application has hardcoded fallback values. (60 seconds for Ping Interval and 500 millseconds for Connect Timeout)
Inside here there are 2 settings:
* PingInterval: Time in seconds to periodically check each server in the pool table (Check for online or offline)
* ConnectTimeout: Time in milliseconds to timeout of the connect. (i.e Determine the server is offline after 500 millseconds)

## pooltable.json
This is a required json file. This is where the Server IP's and Server Ports will be stored. The file is very simple to understand and easy to add or remove servers.

```json
{
  "IPTable": [
    { 
      "IP": "127.0.0.1",
      "Port": 1234 
    },
    { 
      "IP": "127.0.0.2",
      "Port": 5678 
    }
  ]
}
```

### To Add another server to the pool table, simply add a comma after the port } and use this as a template:
```json
{ 
  "IP": "127.0.0.2",
  "Port": 5678 
}
```

## Any Issues?
Don't hesitate to add your issues here. I will try to fix them when they come in and update the git as quickly as possible!

## License
This is licensed under the GNU GPLv3 meaning you can do whatever you want with it with the excemption of making it closed-source.

## Creator
* Myself, Alexander Boulton created this as mentioned above as part of my final year project at Birmingham City University.
* [Nlhomann JSON](https://github.com/nlohmann/json) was used for JSON parsing as it was a simple and open-source JSON library.
All other work inside this project is my own.
