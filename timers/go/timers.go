package main

import (
	"fmt"
	"time"
)

func main() {
	// single-shot timer
	var s time.Duration = 1
	st := time.NewTimer(s * time.Second)

	// periodic timer
	pt := time.NewTicker(time.Second)

	done := make(chan bool)

	// stop demo in 30 sec
	go func() {
		time.Sleep(30 * time.Second)
		done <- true
	}()

	for {
		select {
		case <-done:
			fmt.Println("Demo completed")
			pt.Stop()
			st.Stop()
			return
		case t1 := <-pt.C:
			fmt.Println("periodic timer: ", t1)
		case t2 := <-st.C:
			fmt.Println("single-shot timer: ", t2)
			st = time.NewTimer(s * time.Second)
			s++
		}
	}
}
