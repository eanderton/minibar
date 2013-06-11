drop table if exists users;
create table users (
    username varchar(255) primary key not null,
    role varchar(255) not null,
    password varchar(255) not null
);

insert into users (username,role,password) values ('admin','admin','password');
insert into users (username,role,password) values ('user','user','password');
insert into users (username,role,password) values ('guest','guest','password');

drop table if exists testdata;
create table testdata (
    id bigint primary key,
    data text default '',
    modified datetime default null
);

insert into testdata (data,modified) values ('foo',datetime('NOW'));
insert into testdata (data,modified) values ('bar',datetime('NOW'));
insert into testdata (data,modified) values ('baz',datetime('NOW'));
