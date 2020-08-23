all: build

run:
	GOPATH=$(PWD) go run src/lcd_write.go

build:
	GOPATH=$(PWD) go build src/lcd_write.go

install:
	GOPATH=$(shell pwd) GOBIN=/usr/local/bin/ go install src/lcd_write.go
	../etc/man_installer.sh $(shell pwd)/doc/lcd_write.1

clean:
	rm -f lcd_write

