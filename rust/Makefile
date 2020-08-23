all: build

build:
	cargo build --release

debug:
	cargo build

install_local:
	cargo install --path .

install:
	cargo install --path . --force --no-track --root /usr/local/ --bin simple_lcd_r
	../etc/man_installer.sh $(shell pwd)/doc/simple_lcd_r.1

clean:
	cargo clean

