struct ITEM
{
    1:string url,
    2:bool isExist
}

service Serv{
    list<ITEM> send_url_list(1: list<ITEM> list_url),
}
