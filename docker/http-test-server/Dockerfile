# HTTP server for the http-client tests, built from test/http-test-data.
# Build (from the repo root):
#   docker build -t zorba-http-test -f docker/http-test-server/Dockerfile .
# Run:
#   docker run -d -p 8080:8080 zorba-http-test
# Tests reach it at $ZORBA_HTTP_TEST_SERVER (default localhost:8080).
FROM php:8.3-apache

# Production php.ini: PHP notices must never end up in response bodies.
RUN cp "$PHP_INI_DIR/php.ini-production" "$PHP_INI_DIR/php.ini"

RUN a2enmod cgid auth_basic authn_file \
 && htpasswd -bc /etc/apache2/http-test-data.htpasswd zorba blub

COPY test/http-test-data/docroot /var/www/docroot
COPY test/http-test-data/cgi-bin /var/www/cgi-bin
COPY docker/http-test-server/http-test.conf /etc/apache2/sites-available/http-test.conf

RUN chmod 755 /var/www/cgi-bin/* \
 && a2dissite 000-default \
 && a2ensite http-test \
 && sed -i 's/^Listen 80$/Listen 8080/' /etc/apache2/ports.conf

EXPOSE 8080
