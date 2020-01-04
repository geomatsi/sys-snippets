package main

import (
	"bufio"
	"encoding/hex"
	"fmt"
	"golang.org/x/net/ipv4"
	"gopkg.in/urfave/cli.v1"
	"io"
	"log"
	"net"
	"os"
	"time"
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
					Value: "eth0",
					Usage: "network interface name to bind",
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
			Action: mcastServer,
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
					Value: "eth0",
					Usage: "network interface name to bind",
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

			Action: mcastClient,
		},
	}

	err := mcast.Run(os.Args)
	if err != nil {
		log.Fatal(err)
	}
}

func getInterfaceAddrIPv4(intf *net.Interface) (string, error) {
	var addr string

	addrs, err := intf.Addrs()
	if err != nil {
		return addr, fmt.Errorf("Failed to get list of unicast addresses for %s", intf.Name)
	}

	for _, a := range addrs {
		ip, _, err := net.ParseCIDR(a.String())
		if err != nil {
			continue
		}

		if ip.To4() != nil {
			addr = ip.String()
			break
		}
	}

	return addr, nil
}

func mcastClient(c *cli.Context) error {
	var mstr = c.String("addr")
	var nstr = c.String("bind")
	var body = c.String("message")
	var port = c.Int("port")
	var hops = c.Int("hops")
	var loop = c.Bool("loopback")
	var join = c.Bool("join")
	var cont = c.Bool("cont")

	intf, err := net.InterfaceByName(nstr)
	if err != nil {
		return fmt.Errorf("Interface %s does not exists", nstr)
	}

	maddr, err := net.ResolveUDPAddr("udp4", fmt.Sprintf("%s:%d", mstr, port))
	if err != nil {
		return fmt.Errorf("Invalid IPv4 address/port pair: %s:%d", mstr, port)
	}

	if !maddr.IP.IsMulticast() {
		return fmt.Errorf("IPv4 address %s is not multicast", mstr)
	}

	saddr, err := getInterfaceAddrIPv4(intf)
	if err != nil {
		return err
	}

	if len(saddr) == 0 {
		return fmt.Errorf("Failed to get IPv4 address for %s", intf.Name)
	}

	conn, err := net.ListenPacket("udp4", fmt.Sprintf(fmt.Sprintf("%s:0", saddr)))
	if err != nil {
		return fmt.Errorf("Failed to create UDP socket for %s:0", saddr)
	}
	defer conn.Close()

	pconn := ipv4.NewPacketConn(conn)
	defer pconn.Close()

	if err := pconn.SetMulticastInterface(intf); err != nil {
		return fmt.Errorf("Failed to set multicast interface to %s", intf.Name)
	}

	if join {
		if err := pconn.JoinGroup(intf, maddr); err != nil {
			return fmt.Errorf("Failed to join multicast group %s on %s", maddr.IP, intf.Name)
		}
	}

	if hops > 0 && hops < 255 {
		if err := pconn.SetMulticastTTL(hops); err != nil {
			return fmt.Errorf("Failed to set multicast TTL to %d for %s on %s", hops, maddr.IP, intf.Name)
		}
	}

	if err := pconn.SetMulticastLoopback(loop); err != nil {
		return fmt.Errorf("Failed to set multicast loopback for %s on %s", maddr.IP, intf.Name)
	}

	fmt.Printf("Client ready: addr[%v] intf[%s] port[%d] hops[%d] join[%v] loop[%v] cont[%v]\n",
		maddr.IP, intf.Name, port, hops, join, loop, cont)

	var reader *bufio.Reader = bufio.NewReader(os.Stdin)
	var cnt int = 0

	for {
		if cont {
			time.Sleep(1 * time.Second)
			fmt.Printf("sending packet %d...\n", cnt)
		} else {
			fmt.Println("press enter to send the next multicast packet...")
			_, err = reader.ReadString('\n')
			if err != nil {
				if err == io.EOF {
					fmt.Printf("Client done\n")
					return nil
				}

				return fmt.Errorf("stdio read failure: %s", err)
			}
		}

		_, err = pconn.WriteTo([]byte(fmt.Sprintf("%s:%d\n", body, cnt)), nil, maddr)
		if err != nil {
			return fmt.Errorf("failed to send multicast packet: %s", err)
		}

		cnt++
	}

	return nil
}

func mcastServer(ctx *cli.Context) error {
	var mstr = ctx.String("addr")
	var nstr = ctx.String("bind")
	var dump = ctx.Bool("dump")
	var port = ctx.Int("port")

	intf, err := net.InterfaceByName(nstr)
	if err != nil {
		return fmt.Errorf("Interface %s does not exists", nstr)
	}

	maddr, err := net.ResolveUDPAddr("udp4", fmt.Sprintf("%s:%d", mstr, port))
	if err != nil {
		return fmt.Errorf("Invalid IPv4 address: %s", mstr)
	}

	if !maddr.IP.IsMulticast() {
		return fmt.Errorf("IPv4 address %s is not multicast", mstr)
	}

	conn, err := net.ListenPacket("udp4", fmt.Sprintf("%s:%d", mstr, port))
	if err != nil {
		return fmt.Errorf("Failed to create UDP socket for %s:%d", mstr, port)
	}
	defer conn.Close()

	pconn := ipv4.NewPacketConn(conn)
	defer pconn.Close()

	if err := pconn.JoinGroup(intf, maddr); err != nil {
		return fmt.Errorf("Failed to join multicast group %s on %s", maddr.IP, intf.Name)
	}

	if err := pconn.SetControlMessage(ipv4.FlagDst, true); err != nil {
		return fmt.Errorf("Failed to configure control message for %s on %s", maddr.IP, intf.Name)
	}

	fmt.Printf("Server ready: maddr[%v] intf[%s] port[%d] dump[%v]\n",
		maddr.IP, intf.Name, port, dump)

	var buffer []byte = make([]byte, 1500)

	for {
		size, cm, addr, err := pconn.ReadFrom(buffer)
		if err != nil {
			return fmt.Errorf("Failed to read from %s", pconn)
		}

		if cm.Dst.IsMulticast() {
			if cm.Dst.Equal(maddr.IP) {
				if dump {
					fmt.Printf("received %d bytes from %v:\n%s", size, addr, hex.Dump(buffer[0:size]))
				} else {
					fmt.Printf("received %v bytes from %v: %s\n", size, addr, buffer)
				}
			}
		}
	}

	return nil
}
