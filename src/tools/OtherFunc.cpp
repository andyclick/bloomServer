#include "OtherFunc.h"

string kaiXinPasswordEncode(string parameter)
{
    string ret = "";
    unsigned int begin = 0;
    if ((begin = parameter.find(","))!= string ::npos)
    {
        ret = passworeencode((char *)parameter.substr(0,begin).c_str(),(char *)parameter.substr(begin+1).c_str());
    }
    return ret;
}
string passworeencode(char * passwd, char *key )
{
    return h((char *)en(passwd, key).c_str());
}
void sl(char *s,vector<int >& v,bool w)
{
    int len=strlen(s);
    for(int i=0;i<len;i+=4){
        int tmps = (charCodeAt(s[i]))|(charCodeAt(s[i+1])<<8)|(charCodeAt(s[i+2])<<16)|(charCodeAt(s[i+3])<<24);
        v.push_back(tmps);
        //v[i>>2] = tmps;
    }
    int len1 = v.size();
    if(w){
        v.push_back(len);
        //v[len1] = len;
    }
}
string en(char *p,char *key)
{
    if(!p[0]) return string("");
    vector<int> v; 
    vector<int> k; 
    sl(p,v,true);
    sl(key,k,false);
    if(k.size()<4){
        //k.size()=4;
    }
    int n=v.size()-1;
    int z=v[n],y=v[0],de=2654435769;
    int mx,e,p1;
    int q=(int)::floor(6+52/(n+1));
    int sum=0;
    while(0<q--){
        sum=sum+de&0xffffffff;
        e=(unsigned int )sum>>2&3;
        for(p1=0;p1<n;p1++){
            y=v[p1+1];
            mx=((unsigned int )z>>5^y<<2)+((unsigned int)y>>3^z<<4)^(sum^y)+(k[p1&3^e]^z);
            z=v[p1]=v[p1]+mx&0xffffffff;
        }
        y=v[0];
        mx=((unsigned int )z>>5^y<<2)+((unsigned int )y>>3^z<<4)^(sum^y)+(k[p1&3^e]^z);
        z=v[n]=v[n]+mx&0xffffffff;
    }
    return bh(v);
}
string bh(vector<int> & ar)
{
    char charHex[]={'0','1','2','3','4','5','6','7','8','9','a','b','c','d','e','f'};
    string str="";
    int len=ar.size();
    for(int i=0,tmp=len<<2;i<tmp;i++)
    {
        //str+=charHex[((ar[i>>2]>>(((i&3)<<3)+4))&0xF)]+ charHex[((ar[i>>2]>>((i&3)<<3))&0xF)];
        char a[] ={charHex[((ar[i>>2]>>(((i&3)<<3)+4))&0xF)],0};
        str.append(a);
        char b[] ={charHex[((ar[i>>2]>>((i&3)<<3))&0xF)],0};
        str.append(b);
    }
    return str;
}
string h(char *msg)
{
    string par = msg;
    unsigned int K[] = {0x5a827999,0x6ed9eba1,0x8f1bbcdc,0xca62c1d6};
    char tmp[]={0x080,0};
    par.append(tmp);
    int msglen = par.length();
    char * msgstr = (char *)malloc(sizeof(char)* msglen *4+3);
    memset(msgstr,0,sizeof(char)*msglen *4+3);
    strncpy(msgstr,par.c_str(),msglen);
    msgstr[msglen] = 0;
    float l=(float)par.length()/4+2;
    int N=::ceil(l/16);
    int ** M= (int **)malloc(sizeof(int *) * (N+ 1));
    memset(M,0,sizeof(int *)*(N+1));
    int i = 0;
    for(i=0;i<N;i++){
        M[i] = (int*)malloc(sizeof(int) *16); 
        for(int j=0;j<16;j++){
            M[i][j]=(charCodeAt(msgstr[i*64+j*4])<<24)|(charCodeAt(msgstr[i*64+j*4+1])<<16)|(charCodeAt(msgstr[i*64+j*4+2])<<8)|(charCodeAt(msgstr[i*64+j*4+3]));
        }
    }
    M[N-1][14]=((strlen(msgstr)-1)*8)/::pow(2,32);
    M[N-1][14]=::floor(M[N-1][14]);
    M[N-1][15]=((strlen(msgstr)-1)*8)&0xffffffff;
    if (msgstr) free(msgstr);
    int H0=0x67452301;
    int H1=0xefcdab89;
    int H2=0x98badcfe;
    int H3=0x10325476;
    int H4=0xc3d2e1f0;
    int W[80]={0};
    int a,b,c,d,e;
    for(i=0 ; i<N ; i++){
        for(int t=0;t<16;t++) W[t]=M[i][t];
        for(int t=16;t<80;t++) W[t]=rotl(W[t-3]^W[t-8]^W[t-14]^W[t-16],1);
        a=H0;
        b=H1;
        c=H2;
        d=H3;
        e=H4;
        for(int t=0;t<80;t++){
            int s=::floor(t/20);
            int T=(rotl(a,5)+f(s,b,c,d)+e+K[s]+W[t])&0xffffffff;
            e=d;
            d=c;
            c=rotl(b,30);
            b=a;
            a=T;
        }
        H0=(H0+a)&0xffffffff;
        H1=(H1+b)&0xffffffff;
        H2=(H2+c)&0xffffffff;
        H3=(H3+d)&0xffffffff;
        H4=(H4+e)&0xffffffff;
    }
    for(int j=0;j<N;j++)
    {
        if (M[i]) free(M[i]);
    }
    free(M);
    string ret = "";
    ret.append(tohs(H0));
    ret.append(tohs(H1));
    ret.append(tohs(H2));
    ret.append(tohs(H3));
    ret.append(tohs(H4));
    return ret;
    //return tohs(H0)+tohs(H1)+tohs(H2)+tohs(H3)+tohs(H4);
}
int charCodeAt(char a)
{
    int ret = (int)a & 0xff;
    return ret;
}
int rotl(int x,int  n){
    return(x<<n)|((unsigned)x>>(32-n));
}
int f(int s,int x,int y,int z){
    switch(s)
    {
    case 0:return(x&y)^(~x&z);
    case 1:return x^y^z;
    case 2:return(x&y)^(x&z)^(y&z);
    case 3:return x^y^z;
    }
}
string tohs(int str){
    string s="";
    int v;
    char tmp[4];
    for(int i=7;i>=0;i--){
        v=((unsigned)str>>(i*4))&0xf;
        sprintf(tmp,"%x",v);
        s.append(tmp);
    }
    return s;
}
