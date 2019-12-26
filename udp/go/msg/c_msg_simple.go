package main

import (
	"bufio"
	"fmt"
	"io"
	"net"
	"os"
)

func main() {
	conn, err := net.ListenPacket("udp4", ":0")
	if err != nil {
		fmt.Printf("err: ListenPacket: %s\n", err)
		os.Exit(-1)
	}

	defer conn.Close()

	dst, err := net.ResolveUDPAddr("udp", "127.0.0.1:20001")
	if err != nil {
		fmt.Printf("err: resolve UDP addr: %s\n", err)
		os.Exit(-1)
	}

	var reader *bufio.Reader
	var str string
	var num int

	reader = bufio.NewReader(os.Stdin)

	for {
		fmt.Print("Enter message: ")
		str, err = reader.ReadString('\n')
		if err != nil {
			if err == io.EOF {
				fmt.Printf("client %s: done\n", conn.LocalAddr())
				os.Exit(-1)
			}

			fmt.Printf("err: stdio read: %s\n", err)
			os.Exit(-1)
		}

		fmt.Printf("xmit: %s", str)
		num, err = conn.WriteTo([]byte(str), dst)
		if err != nil {
			fmt.Printf("err: write: %s\n", err)
			os.Exit(-1)
		}

		if num != len(str) {
			fmt.Printf("warn: sent %v != %v bytes\n", num, len(str))
		}
	}
}
