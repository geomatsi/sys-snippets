package main

import (
	"fmt"
	//"golang.org/x/net/ipv4"
	"gopkg.in/urfave/cli.v1"
	//"net"
	"log"
	//"errors"
	"os"
)

func main() {
	mcast := cli.NewApp()
	mcast.Name = "mcast"
	mcast.Usage = "IPv4 multicast test tool"
	mcast.Version = "0.1"

	mcast.Commands = []cli.Command{
		{
			Name:    "server",
			Aliases: []string{"s", "srv"},
			Usage:   "Multicast server test tool",
			Flags: []cli.Flag{
				cli.StringFlag{
					Name:  "a, addr",
					Value: "224.0.0.1",
					Usage: "multicast IPv4 address",
				},
				cli.StringFlag{
					Name:  "b, bind",
					Value: "0.0.0.0",
					Usage: "network interface name or IPv4 address to bind",
				},
				cli.IntFlag{
					Name:  "p, port",
					Value: 9999,
					Usage: "multicast port, default value is 9999",
				},
				cli.BoolFlag{
					Name:  "d, dump",
					Usage: "multicast port, default value is 9999",
				},
			},
			Action: func(c *cli.Context) error {
				var maddr = c.String("addr")
				var saddr = c.String("bind")
				var dump = c.Bool("dump")
				var port = c.Int("port")

				fmt.Printf("server: maddr[%s] saddr[%s] port[%v] dump[%v]\n",
					maddr, saddr, port, dump)
				return nil
			},
		},
		{
			Name:    "client",
			Aliases: []string{"c", "cli"},
			Usage:   "Multicast clienttest tool",
			Flags: []cli.Flag{
				cli.StringFlag{
					Name:  "a, addr",
					Value: "224.0.0.1",
					Usage: "multicast IPv4 address",
				},
				cli.StringFlag{
					Name:  "b, bind",
					Value: "0.0.0.0",
					Usage: "network interface name or IPv4 address to bind",
				},
				cli.IntFlag{
					Name:  "p, port",
					Value: 9999,
					Usage: "multicast port, default value is 9999",
				},
				cli.StringFlag{
					Name:  "m, message",
					Value: "test",
					Usage: "client multicast payload message template",
				},
				cli.IntFlag{
					Name:  "hops",
					Value: 255,
					Usage: "multicast packet lifetime",
				},
				cli.BoolFlag{
					Name:  "loopback",
					Usage: "enable client transmission loopback",
				},
				cli.BoolFlag{
					Name:  "join",
					Usage: "enable client membership in multicast group",
				},
				cli.BoolFlag{
					Name:  "cont",
					Usage: "enable client continuous mode",
				},
			},

			Action: func(c *cli.Context) error {
				var maddr = c.String("addr")
				var saddr = c.String("bind")
				var body = c.String("message")
				var port = c.Int("port")
				var hops = c.Int("hops")
				var loop = c.Bool("loopback")
				var join = c.Bool("join")
				var cont = c.Bool("cont")

				fmt.Printf("client: maddr[%s] saddr[%s] port[%v] message[%s] hops[%v] loop[%v] join[%v] cont[%v]\n",
					maddr, saddr, port, body, hops, loop, join, cont)

				return nil
			},
		},
	}

	err := mcast.Run(os.Args)
	if err != nil {
		log.Fatal(err)
	}
}
