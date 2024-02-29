#!/bin/bash
rm release_version/packet/*


export version1_new=1-1-6-1-48

#zip -q -j ./release_version/packet/1-1-0-1-3-1906-new.zip ./release_version/app1_1906_new/*
#zip -q -j ./release_version/packet/1-1-0-1-1-2108-old.zip ./release_version/app1_2108_old/*


cp ./app1_1906_new/mb_usbshare_n ./release_version/app1_1906_new
cp ./app1_1908_new/mb_usbshare_n ./release_version/app1_1908_new
cp ./app1_1910_new/mb_usbshare_n ./release_version/app1_1910_new
cp ./app1_2106_new/mb_usbshare_n ./release_version/app1_2106_new
cp ./app1_2108_new/mb_usbshare_n ./release_version/app1_2108_new
cp ./app1_2110_new/mb_usbshare_n ./release_version/app1_2110_new
cp ./app1_2202_new/mb_usbshare_n ./release_version/app1_2202_new
cp ./app1_2204_new/mb_usbshare_n ./release_version/app1_2204_new
cp ./app1_2212_new/mb_usbshare_n ./release_version/app1_2212_new
cp ./app1_2216_new/mb_usbshare_n ./release_version/app1_2216_new
cp ./app1_2306_new/mb_usbshare_n ./release_version/app1_2306_new

cp ./app1_1201_new/mb_usbshare_n ./release_version/app1_1201_new
							   
cp ./release_version/taxlib/mb_taxlib/lib-arm-new/libmb_taxlib.so ./release_version/app1_1906_new
cp ./release_version/taxlib/mb_taxlib/lib-arm-new/libmb_taxlib.so ./release_version/app1_1908_new
cp ./release_version/taxlib/mb_taxlib/lib-arm-new/libmb_taxlib.so ./release_version/app1_1910_new
cp ./release_version/taxlib/mb_taxlib/lib-arm-new/libmb_taxlib.so ./release_version/app1_2106_new
cp ./release_version/taxlib/mb_taxlib/lib-arm-new/libmb_taxlib.so ./release_version/app1_2108_new
cp ./release_version/taxlib/mb_taxlib/lib-arm-new/libmb_taxlib.so ./release_version/app1_2110_new
cp ./release_version/taxlib/mb_taxlib/lib-arm-new/libmb_taxlib.so ./release_version/app1_2202_new
cp ./release_version/taxlib/mb_taxlib/lib-arm-new/libmb_taxlib.so ./release_version/app1_2204_new
cp ./release_version/taxlib/mb_taxlib/lib-arm-new/libmb_taxlib.so ./release_version/app1_2212_new
cp ./release_version/taxlib/mb_taxlib/lib-arm-new/libmb_taxlib.so ./release_version/app1_2216_new
cp ./release_version/taxlib/mb_taxlib/lib-arm-new/libmb_taxlib.so ./release_version/app1_2306_new

cp /usr/local/lib-linux64-tassl/libmb_taxlib.so ./release_version/app1_1201_new


cp ./release_version/taxlib/mb_clib/lib-arm/libmb_clib.so ./release_version/app1_1906_new
cp ./release_version/taxlib/mb_clib/lib-arm/libmb_clib.so ./release_version/app1_1908_new
cp ./release_version/taxlib/mb_clib/lib-arm/libmb_clib.so ./release_version/app1_1910_new
cp ./release_version/taxlib/mb_clib/lib-arm/libmb_clib.so ./release_version/app1_2106_new
cp ./release_version/taxlib/mb_clib/lib-arm/libmb_clib.so ./release_version/app1_2108_new
cp ./release_version/taxlib/mb_clib/lib-arm/libmb_clib.so ./release_version/app1_2110_new
cp ./release_version/taxlib/mb_clib/lib-arm/libmb_clib.so ./release_version/app1_2202_new
cp ./release_version/taxlib/mb_clib/lib-arm/libmb_clib.so ./release_version/app1_2204_new
cp ./release_version/taxlib/mb_clib/lib-arm/libmb_clib.so ./release_version/app1_2212_new
cp ./release_version/taxlib/mb_clib/lib-arm/libmb_clib.so ./release_version/app1_2216_new
cp ./release_version/taxlib/mb_clib/lib-arm/libmb_clib.so ./release_version/app1_2306_new


#npc升级的时候放开，并且更换目录先的npc可执行文件
#cp ./app1_1906_new/npc ./release_version/app1_1906_new
#cp ./app1_1906_new/npc ./release_version/app1_1908_new
#cp ./app1_1906_new/npc ./release_version/app1_1910_new
#cp ./app1_1906_new/npc ./release_version/app1_2106_new
#cp ./app1_1906_new/npc ./release_version/app1_2108_new
#cp ./app1_1906_new/npc ./release_version/app1_2110_new
#cp ./app1_1906_new/npc ./release_version/app1_2202_new
#cp ./app1_1906_new/npc ./release_version/app1_2204_new
#cp ./app1_1906_new/npc ./release_version/app1_2212_new
#cp ./app1_1906_new/npc ./release_version/app1_2216_new
#cp ./app1_1906_new/npc ./release_version/app1_2306_new



zip -q -j ./release_version/packet/$version1_new-1906-new.zip ./release_version/app1_1906_new/*
zip -q -j ./release_version/packet/$version1_new-1908-new.zip ./release_version/app1_1908_new/*
zip -q -j ./release_version/packet/$version1_new-1910-new.zip ./release_version/app1_1910_new/*
zip -q -j ./release_version/packet/$version1_new-2106-new.zip ./release_version/app1_2106_new/*
zip -q -j ./release_version/packet/$version1_new-2108-new.zip ./release_version/app1_2108_new/*
zip -q -j ./release_version/packet/$version1_new-2110-new.zip ./release_version/app1_2110_new/*
zip -q -j ./release_version/packet/$version1_new-2202-new.zip ./release_version/app1_2202_new/*
zip -q -j ./release_version/packet/$version1_new-2204-new.zip ./release_version/app1_2204_new/*
zip -q -j ./release_version/packet/$version1_new-2212-new.zip ./release_version/app1_2212_new/*
zip -q -j ./release_version/packet/$version1_new-2216-new.zip ./release_version/app1_2216_new/*
zip -q -j ./release_version/packet/$version1_new-2306-new.zip ./release_version/app1_2306_new/*

zip -q -j ./release_version/packet/$version1_new-1201-new.zip ./release_version/app1_1201_new/*
   							
#rm release_version/packet-6-66-2.zip
#zip -q -j release_version/packet-6-66-1.zip release_version/packet/*
