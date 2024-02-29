#!/bin/bash
make clean -f buildall
rm release_version/packet/*
#cp appmain/_app.* app1_1906_new/
cp app1_1906_new/_app.* app1_1908_new/
cp app1_1906_new/_app.* app1_1910_new/
cp app1_1906_new/_app.* app1_2106_new/
cp app1_1906_new/_app.* app1_2108_new/
cp app1_1906_new/_app.* app1_2110_new/
cp app1_1906_new/_app.* app1_2202_new/
cp app1_1906_new/_app.* app1_2204_new/
cp app1_1906_new/_app.* app1_2212_new/
cp app1_1906_new/_app.* app1_2216_new/
cp app1_1906_new/_app.* app1_2306_new/

