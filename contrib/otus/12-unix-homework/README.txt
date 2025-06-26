
1. download curl sources(curl ver 8_6_0)
2. cd downloaded curl sources
3. mkdir private_curl_build 
4. ./configure --disable-tftp --disable-smtp --disable-rtsp --disable-smb --disable-pop3 --disable-mqtt --disable-ldap --disable-imap --disable-gopher --disable-ftp --disable-file --disable-dict --without-libpsl --with-openssl --prefix=/home/vasilisk/otus_c_course/otus_c_homework/12-unix-homework/curl-curl-8_6_0/private_curl_build
5. make
6. make install
7. env LD_LIBRARY_PATH="$LD_LIBRARY_PATH:/home/vasilisk/otus_c_course/otus_c_homework/12-unix-homework/curl-curl-8_6_0/private_curl_build/lib" curl --version