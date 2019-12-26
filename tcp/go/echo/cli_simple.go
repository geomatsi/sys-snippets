package main

import (
	"bufio"
	"fmt"
	"io"
	"net"
	"os"
)

func main() {
	conn, err := net.Dial("tcp", "localhost:10000")
	if err != nil {
		fmt.Printf("err: listen: %s\n", err)
		return
	}

	fmt.Printf("client %s connected\n", conn.LocalAddr())
	defer conn.Close()

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
				return
			}

			fmt.Printf("err: stdio read: %s\n", err)
			return
		}

		fmt.Printf("xmit: %s", str)
		num, err = conn.Write([]byte(str))
		if err != nil {
			fmt.Printf("err: write: %s\n", err)
			return
		}

		if num != len(str) {
			fmt.Printf("warn: sent %v != %v bytes\n", num, len(str))
		}

		str, err = bufio.NewReader(conn).ReadString('\n')
		if err != nil {
			if err == io.EOF {
				fmt.Printf("server %s closed connection\n", conn.RemoteAddr())
				return
			}

			fmt.Printf("err: server read: %s\n", err)
			return
		}

		fmt.Printf("recv: %s\n", str)
	}
}
