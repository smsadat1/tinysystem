package main

import (
	"log"
	"net/http"
)

func main() {
	http.HandleFunc("/view/", ViewHandler)
	http.HandleFunc("/edit/", EditHandler)
	http.HandleFunc("/save/", SaveHandler)
	err := http.ListenAndServe(":8080", nil)
	log.Fatal(err)
}
