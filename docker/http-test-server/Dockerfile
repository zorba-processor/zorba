# Local replacement for the retired zorbatest.28.io:8080 test server.
# Content and configuration come from test/http-test-data (see its README).
# Build (from the repo root):
#   docker build -t zorba-http-test -f docker/http-test-server/Dockerfile .
# Tests name the host explicitly, so run it on a Docker network with the
# alias zorbatest.28.io (see docker/test-with-http-server.sh).
FROM php:8.3-apache

RUN a2enmod cgid auth_basic authn_file \
 && htpasswd -bc /etc/apache2/http-test-data.htpasswd zorba blub

COPY test/http-test-data/docroot /var/www/docroot
COPY test/http-test-data/cgi-bin /var/www/cgi-bin
COPY docker/http-test-server/zorbatest.conf /etc/apache2/sites-available/zorbatest.conf

RUN chmod 755 /var/www/cgi-bin/* \
 && a2dissite 000-default \
 && a2ensite zorbatest \
 && sed -i 's/^Listen 80$/Listen 8080/' /etc/apache2/ports.conf

EXPOSE 8080
