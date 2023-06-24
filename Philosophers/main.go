package main

import (
	"fmt"
	"log"
	"os"
	"sync"
	"time"
)

var eatingDuration = 2000 * time.Millisecond
var canSurviveWithoutFoodDuration = 8 * eatingDuration
var startTime time.Time

type Philosopher struct {
	index         int
	lastAte       time.Time
	leftFork      *sync.Mutex
	rightFork     *sync.Mutex
	waiterChannel chan bool
}

type Waiter struct {
	philosopherChannels [5]chan bool
}

func checkError(err error) {
	if err != nil {
		log.Fatal(err)
	}
}

func printMesage(messageType string, number int) {
	fmt.Println(time.Now().Sub(startTime).Milliseconds(), "Philosopher", number, messageType)
	if messageType == "dead" {
		os.Exit(1)
	}
}

func startMonitoringPhilosophers(p *[]Philosopher) {
	for {
		for _, value := range *p {
			if (time.Now().Sub(value.lastAte)) > canSurviveWithoutFoodDuration {
				printMesage("dead", value.index)
			}
		}

	}
}

func (w *Waiter) startServing() {
	for i := 0; ; i = (i + 1) % 5 {
		w.philosopherChannels[i] <- true
		w.philosopherChannels[(i+2)%5] <- true
		_ = <-w.philosopherChannels[i]
		_ = <-w.philosopherChannels[(i+2)%5]
	}
}

func (p *Philosopher) startEating() {
	for {
		_ = <-p.waiterChannel
		p.leftFork.Lock()
		printMesage("took left fork", p.index)
		p.rightFork.Lock()
		printMesage("took right fork", p.index)
		printMesage("eating", p.index)
		time.Sleep(eatingDuration)
		p.lastAte = time.Now()
		p.leftFork.Unlock()
		p.rightFork.Unlock()
		p.waiterChannel <- true
		printMesage("thinking", p.index)
	}
}

func main() {
	var wg sync.WaitGroup
	var mutex = make([]sync.Mutex, 5)
	var philosophers = make([]Philosopher, 5)

	var commandChannels [5]chan bool
	for i := range commandChannels {
		commandChannels[i] = make(chan bool, 1)
	}
	startTime = time.Now()

	for i := 0; i < 5; i++ {
		philosophers[i] = Philosopher{
			index:         i + 1,
			leftFork:      &mutex[i],
			rightFork:     &mutex[(i+1)%5],
			lastAte:       startTime,
			waiterChannel: commandChannels[i],
		}
	}

	var waiter = Waiter{
		philosopherChannels: commandChannels,
	}

	for i := 0; i < 5; i++ {
		wg.Add(1)
		go philosophers[i].startEating()
		defer wg.Done()
	}

	go startMonitoringPhilosophers(&philosophers)
	go waiter.startServing()

	wg.Wait()
}
