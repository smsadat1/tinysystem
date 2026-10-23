package main

import (
	"html/template"
	"net/http"
)

func renderTemplate(w http.ResponseWriter, tmpl string, p *Page) {

	t, err := template.ParseFiles(tmpl)
	if err != nil {
		http.Error(w, err.Error(), http.StatusInternalServerError)
		return
	}

	err = t.Execute(w, p)
	if err != nil {
		http.Error(w, err.Error(), http.StatusInternalServerError)
	}
}

func ViewHandler(w http.ResponseWriter, r *http.Request) {

	title := r.URL.Path[len("/view/"):]

	p, err := LoadPage(title)
	if err != nil {
		renderTemplate(w, "notfound.html", nil)
	}

	renderTemplate(w, "view.html", p)
}

func EditHandler(w http.ResponseWriter, r *http.Request) {

	title := r.URL.Path[len("/view/"):]

	p, err := LoadPage(title)
	if err != nil {
		p = &Page{Title: title}
	}

	renderTemplate(w, "edit.html", p)
}

func SaveHandler(w http.ResponseWriter, r *http.Request) {

	title := r.URL.Path[len("/save/"):]
	body := r.FormValue("body")

	p := &Page{Title: title, Body: []byte(body)}
	p.Save()

	http.Redirect(w, r, "/view/"+title, http.StatusFound)
}
