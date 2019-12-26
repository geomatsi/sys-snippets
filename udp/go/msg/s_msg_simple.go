package main

import (
	"fmt"
	"net"
	"os"
)

func main() {
	bind, err := net.ResolveUDPAddr("udp", "127.0.0.1:20001")
	if err != nil {
		fmt.Printf("err: resolve UDP addr: %s\n", err)
		os.Exit(-1)
	}

	sock, err := net.ListenUDP("udp4", bind)
	if err != nil {
		fmt.Printf("err: listen UDP: %s\n", err)
		os.Exit(-1)
	}

	defer sock.Close()

	var buffer []byte = make([]byte, 256)

	for {
		size, addr, err := sock.ReadFromUDP(buffer)
		if err != nil {
			fmt.Printf("err: read from UDP: %s\n", err)
			os.Exit(-1)
		}

		fmt.Printf("received %v bytes from %v: %s\n", size, addr, buffer)
	}
}
