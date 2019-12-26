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

	nc := make(chan string)
	cc := make(chan string)

	go handleCli(cc)
	go handleNet(nc, conn)

	for {
		select {
		case str, more := <-cc:
			if more == false {
				return
			}

			fmt.Printf("xmit: %s", str)
			num, err := conn.Write([]byte(str))
			if err != nil {
				fmt.Printf("err: write: %s\n", err)
				return
			}

			if num != len(str) {
				fmt.Printf("warn: sent %v != %v bytes\n", num, len(str))
			}

		case str, more := <-nc:
			if more == false {
				return
			}

			fmt.Printf("recv: %s\n", str)
		}
	}
}

func handleCli(cc chan string) {
	reader := bufio.NewReader(os.Stdin)

	for {
		fmt.Print("Enter message: ")
		str, err := reader.ReadString('\n')
		if err != nil {
			if err != io.EOF {
				fmt.Printf("err: stdio read: %s\n", err)
			}

			close(cc)
			return
		}

		cc <- str
	}
}

func handleNet(nc chan string, conn net.Conn) {
	for {
		str, err := bufio.NewReader(conn).ReadString('\n')
		if err != nil {
			fmt.Printf("connection closed to server %s\n", conn.RemoteAddr())
			close(nc)
			return
		}

		nc <- str
	}
}
