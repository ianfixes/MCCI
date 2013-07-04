
drop table if exists signature;

create table signature(
    signature_id integer not null,
    signature text not null unique,

    primary key (signature_id)
);


drop table if exists revision;

create table revision(
    var_id integer not null,
    signature_id integer not null,
    revision integer not null,

    primary key (var_id),
    foreign key (signature_id) references signature(signature_id)
);


