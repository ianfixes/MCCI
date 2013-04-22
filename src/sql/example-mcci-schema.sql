
drop table if exists var;

create table var(
    var_id integer not null,
    name text not null,
    category_id integer,
    enabled boolean not null,
    protobuf_id integer,
    unit integer,

    primary key (var_id)
);


drop table if exists category;

create table category(
    category_id integer not null,
    name text not null,
    parent_category_id integer,
    
    primary key (category_id),
    foreign key (parent_category_id) references category(category_id)
);


drop table if exists tag;

create table tag(
    tag_id integer not null,
    name text not null,

    primary key (tag_id)        
);


drop table if exists var_tag;

create table var_tag(
    var_id integer not null,
    tag_id integer not null,

    primary key (var_id, tag_id),
    foreign key (var_id) references var(var_id),
    foreign key (tag_id) references tag(tag_id)
);


insert into category(category_id, name) values(1, 'Primitives');

insert into var(name, category_id, enabled) values('Double', 1, 1);
insert into var(name, category_id, enabled) values('String', 1, 1);