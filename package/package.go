package kle_reader

import (
	"github.com/jurgen-kluft/xbase/package"
	"github.com/jurgen-kluft/xcode/denv"
	"github.com/jurgen-kluft/xentry/package"
	"github.com/jurgen-kluft/xjsmn/package"
	"github.com/jurgen-kluft/xunittest/package"
)

// GetPackage returns the package object of 'kle_reader'
func GetPackage() *denv.Package {
	// Dependencies
	xunittestpkg := xunittest.GetPackage()
	xentrypkg := xentry.GetPackage()
	xbasepkg := xbase.GetPackage()
	xjsmnpkg := xjsmn.GetPackage()

	// The main (kle_reader) package
	mainpkg := denv.NewPackage("kle_reader")
	mainpkg.AddPackage(xunittestpkg)
	mainpkg.AddPackage(xentrypkg)
	mainpkg.AddPackage(xbasepkg)

	// 'kle_reader' library
	mainlib := denv.SetupDefaultCppLibProject("kle_reader", "github.com\\jurgen-kluft\\kle_reader")
	mainlib.Dependencies = append(mainlib.Dependencies, xbasepkg.GetMainLib())
	mainlib.Dependencies = append(mainlib.Dependencies, xjsmnpkg.GetMainLib())

	// 'kle_reader' unittest project
	maintest := denv.SetupDefaultCppTestProject("kle_reader_test", "github.com\\jurgen-kluft\\kle_reader")
	maintest.Dependencies = append(maintest.Dependencies, xunittestpkg.GetMainLib())
	maintest.Dependencies = append(maintest.Dependencies, xentrypkg.GetMainLib())
	maintest.Dependencies = append(maintest.Dependencies, xbasepkg.GetMainLib())
	maintest.Dependencies = append(maintest.Dependencies, xjsmnpkg.GetMainLib())
	maintest.Dependencies = append(maintest.Dependencies, mainlib)

	mainpkg.AddMainLib(mainlib)
	mainpkg.AddUnittest(maintest)
	return mainpkg
}
