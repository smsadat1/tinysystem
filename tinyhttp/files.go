package main

import (
	"os"
)

type Page struct {
	Title string
	Body  []byte
}

// Todo: Migrate from direct filesystem API to TinyObj http APIs when implemented

func (p *Page) Save() error {
	filename := p.Title + ".txt"
	return os.WriteFile(filename, p.Body, 0600)
}

func (p *Page) Rename(newname string) error {
	return os.Rename(p.Title, newname+".txt")
}

func (p *Page) Delete() error {
	filename := p.Title + ".txt"
	return os.Remove(filename)
}

func LoadPage(title string) (*Page, error) {
	filename := title + ".txt"

	body, err := os.ReadFile(filename)
	if err != nil {
		return &Page{}, err
	}

	return &Page{Title: filename, Body: body}, nil
}
