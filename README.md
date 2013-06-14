Minibar
=======

A minimal, database-to-REST, CGI application. A JSON configuration file is all that's needed to bind a database to RESTful URL routes.  As long as your web client can send and receive JSON, it can talk to Minibar.

What Minibar is For
===================

Embedded applications, prototyping environments, and minimalistic web engineers. The idea behind Minibar is to provide the bare minimum for moving database data in and out of RESTful web queries.

What Minibar is Not For
=======================

Minibar is a minimal solution.  What it doesn't do is provide a complete business logic layer to sit in between your web server and your database.  Instead, Minibar takes the shortest possible route: a datatype-validated mapping between HTTP requests and SQL queries.

Frontend Support
================

FastCGI is the currently supported frontend.  FastCGI enjoys support under Apache, Lighttpd, and more.

Backend Support
===============

Right now, SQLite3 is the only supported backend.  Other databases will be supported in the future.

Dependencies
============

The project depends on the following external dependencies to compile:

* libpthread
* libfcgi

