#!/bin/bash

rm -rf ./release_version/taxlib/mb_taxlib/*

cp -rf ../../mb_taxlib/mb_taxlib_v1.1-6-1-48/lib-arm-new ./release_version/taxlib/mb_taxlib/


cp -rf ../../mb_taxlib/mb_taxlib_v1.1-6-1-48/mb_taxlib ./release_version/taxlib/mb_taxlib/


rm -rf /usr/local/release-include-lib/include-6-1-48

rm -rf /usr/local/release-include-lib/lib-arm-tassl-6-1-48

cp -rf /usr/local/release-include-lib/include-6-1-15 /usr/local/release-include-lib/include-6-1-48


cp -rf /usr/local/release-include-lib/lib-arm-tassl-6-1-15 /usr/local/release-include-lib/lib-arm-tassl-6-1-48


rm -rf /usr/local/release-include-lib/include-6-1-48/mb_clib
rm -rf /usr/local/release-include-lib/include-6-1-48/mb_taxlib


cp -rf /mnt/share/usbshare/mb_lib/mb_main/mb_usbshare-X.1-6-1-48/release_version/taxlib/mb_clib/mb_clib /usr/local/release-include-lib/include-6-1-48/
cp -rf /mnt/share/usbshare/mb_lib/mb_main/mb_usbshare-X.1-6-1-48/release_version/taxlib/mb_taxlib/mb_taxlib /usr/local/release-include-lib/include-6-1-48/

rm -rf /usr/local/release-include-lib/lib-arm-tassl-6-1-48/libmb_clib.so
cp /mnt/share/usbshare/mb_lib/mb_main/mb_usbshare-X.1-6-1-48/release_version/taxlib/mb_clib/lib-arm/libmb_clib.so /usr/local/release-include-lib/lib-arm-tassl-6-1-48/


rm -rf /usr/local/release-include-lib/lib-arm-tassl-6-1-48/libmb_taxlib.so
cp /mnt/share/usbshare/mb_lib/mb_main/mb_usbshare-X.1-6-1-48/release_version/taxlib/mb_taxlib/lib-arm-new/libmb_taxlib.so /usr/local/release-include-lib/lib-arm-tassl-6-1-48/


make -f buildall
