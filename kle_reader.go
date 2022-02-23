package main

import (
	pkg "github.com/jurgen-kluft/kle_reader/package"
	"github.com/jurgen-kluft/xcode"
)

func main() {
	xcode.Init()
	xcode.Generate(pkg.GetPackage())
}
