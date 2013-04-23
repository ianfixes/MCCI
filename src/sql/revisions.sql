
drop table if exists revision;

create table revision(
    var_id integer not null,
    revision integer not null,

    primary key (var_id)
);


drop table if exists signature;

create table signature(
    text signature not null
);