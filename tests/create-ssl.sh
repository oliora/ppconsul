#!/bin/bash
# Based on
# https://www.digitalocean.com/community/tutorials/how-to-secure-consul-with-tls-encryption-on-ubuntu-14-04

set -ex

DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"
NEWCERTTOP="$(pwd)/tls/certs"
CATOP="$(pwd)/tls/ca"

function create_ca {
    [ ! -d $CATOP ] || (echo "CA directory already exists" && exit 1)
    mkdir -p $CATOP/newcerts
    cp $DIR/ca.cfg $CATOP/ca.cfg
    touch $CATOP/certindex
    echo "000a" > $CATOP/serial
    openssl req -x509 -newkey rsa:2048 -nodes -keyout $CATOP/privkey.pem -out $CATOP/ca.cert -subj "/C=NL/L=The Hague/O=ppconsul-test/CN=ca"
}

# Create CA authority if doesn't exist
[ -d $CATOP ] || create_ca

# Generate new certificate
mkdir -p $NEWCERTTOP

openssl req -newkey rsa:1024 -nodes -out $NEWCERTTOP/consul.csr -keyout $NEWCERTTOP/consul.key -config $CATOP/ca.cfg -subj "/C=NL/L=The Hague/O=ppconsul-test/CN=localhost"

# Can't make relative dirs in config work
(cd $CATOP && openssl ca -batch -config $CATOP/ca.cfg -notext -in $NEWCERTTOP/consul.csr -cert $CATOP/ca.cert -keyfile $CATOP/privkey.pem -out $NEWCERTTOP/consul.cert)

rm $NEWCERTTOP/consul.csr

# On macOS we have to use PKCS12 format with password
openssl pkcs12 -export -in $NEWCERTTOP/consul.cert -inkey $NEWCERTTOP/consul.key -out $NEWCERTTOP/consul.p12 -passout pass:thepassword
