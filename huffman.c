#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define SYM 256

typedef struct N{
    int sym;
    unsigned long f;
    struct N *l, *r;
} N;

N *nodes[SYM*2];
int Nn = 0;

N* mk(int s,unsigned long f,N*L,N*R){
    N* n = malloc(sizeof(N));
    n->sym = s; n->f = f;
    n->l = L; n->r = R;
    nodes[Nn++] = n;
    return n;
}

void push(N**h,int *hs,N*x){
    h[(*hs)++] = x;
    for(int i=*hs-1;i;i=(i-1)/2)
        if(h[(i-1)/2]->f > h[i]->f){
            N*t=h[(i-1)/2]; h[(i-1)/2]=h[i]; h[i]=t;
        } else break;
}

N* pop(N**h,int *hs){
    N* r=h[0];
    h[0]=h[--(*hs)];
    int i=0;
    for(;;){
        int l=2*i+1, rgt=2*i+2, sm=i;
        if(l<*hs && h[l]->f < h[sm]->f) sm=l;
        if(rgt<*hs && h[rgt]->f < h[sm]->f) sm=rgt;
        if(sm==i) break;
        N*t=h[i]; h[i]=h[sm]; h[sm]=t;
        i=sm;
    }
    return r;
}

char codes[SYM][512];

void build(N*root,char*s,int d){
    if(!root) return;
    if(!root->l && !root->r){
        s[d]=0; strcpy(codes[root->sym], s);
        return;
    }
    s[d]='0'; build(root->l, s, d+1);
    s[d]='1'; build(root->r, s, d+1);
}

long fsize(const char*p){
    FILE *f=fopen(p,"rb");
    if(!f) return -1;
    fseek(f,0,SEEK_END);
    long r=ftell(f);
    fclose(f);
    return r;
}

void compress_file(const char *in,const char *out){
    unsigned long freq[SYM]={0};
    FILE *fi=fopen(in,"rb");
    if(!fi){ printf("Input error\n"); return; }

    int c;
    while((c=fgetc(fi))!=EOF) freq[c]++;

    Nn=0;
    N*heapArr[SYM];
    int hs=0;

    for(int i=0;i<SYM;i++)
        if(freq[i])
            push(heapArr,&hs, mk(i,freq[i],NULL,NULL));

    if(hs==0){ fclose(fi); printf("Empty file!\n"); return; }

    while(hs>1){
        N*a=pop(heapArr,&hs), *b=pop(heapArr,&hs);
        push(heapArr,&hs, mk(-1,a->f+b->f,a,b));
    }

    N *root = pop(heapArr,&hs);

    char tmp[512];
    build(root,tmp,0);

    FILE *fo=fopen(out,"wb");
    int count=0;
    for(int i=0;i<SYM;i++)
        if(freq[i]) count++;

    fwrite(&count, sizeof(int), 1, fo);

    for(int i=0;i<SYM;i++)
        if(freq[i]){
            unsigned char s=i;
            fwrite(&s,1,1,fo);
            fwrite(&freq[i], sizeof(unsigned long),1,fo);
        }

    fseek(fi,0,SEEK_SET);
    unsigned char bout=0;
    int pos=7;

    while((c=fgetc(fi))!=EOF){
        char *cd=codes[c];
        for(int i=0; cd[i]; i++){
            if(cd[i]=='1') bout |= (1<<pos);
            pos--;
            if(pos<0){
                fwrite(&bout,1,1,fo);
                bout=0;
                pos=7;
            }
        }
    }

    if(pos!=7)
        fwrite(&bout,1,1,fo);

    fclose(fi);
    fclose(fo);
    printf("Compressed -> %s\n", out);
}

void decompress_file(const char *in,const char *out){
    FILE *fi=fopen(in,"rb");
    if(!fi){ printf("Cannot open compressed file.\n"); return; }

    int count;
    fread(&count,sizeof(int),1,fi);

    unsigned long freq[SYM]={0};
    for(int i=0;i<count;i++){
        unsigned char s;
        unsigned long f;
        fread(&s,1,1,fi);
        fread(&f,sizeof(unsigned long),1,fi);
        freq[s]=f;
    }

    Nn=0;
    N*heapArr[SYM];
    int hs=0;

    for(int i=0;i<SYM;i++)
        if(freq[i])
            push(heapArr,&hs, mk(i,freq[i],NULL,NULL));

    if(hs==1){
        N*single=pop(heapArr,&hs);
        N*rt=mk(-1,single->f,single,NULL);
        push(heapArr,&hs,rt);
    }

    while(hs>1){
        N*a=pop(heapArr,&hs), *b=pop(heapArr,&hs);
        push(heapArr,&hs, mk(-1,a->f+b->f,a,b));
    }

    N*root = pop(heapArr,&hs);
    N*cur = root;

    FILE *fo=fopen(out,"wb");

    unsigned long total=0;
    for(int i=0;i<SYM;i++) total+=freq[i];

    unsigned long dec=0;
    int byte;

    while(dec<total && (byte=fgetc(fi))!=EOF){
        for(int b=7;b>=0 && dec<total; b--){
            int bit=(byte>>b)&1;
            cur = bit ? cur->r : cur->l;
            if(!cur->l && !cur->r){
                fputc(cur->sym,fo);
                cur=root;
                dec++;
            }
        }
    }

    fclose(fi);
    fclose(fo);
    printf("Decompressed -> %s\n", out);
}

void show_table(const char *orig,const char *comp,const char *recon){
    long a=fsize(orig), b=fsize(comp), c=fsize(recon);

    printf("\n+----------------------+---------------+\n");
    printf("| File                 | Size (bytes)  |\n");
    printf("+----------------------+---------------+\n");
    printf("| Original             | %13ld |\n", a);
    printf("| Compressed (.huff)   | %13ld |\n", b);
    printf("| Reconstructed        | %13ld |\n", c);
    printf("+----------------------+---------------+\n\n");

    if(a>0)
        printf("Compression ratio: %.2f%%\n", (double)b/a * 100.0);
}

int main(){
    char in[256];
    printf("Enter input text file: ");
    scanf("%255s", in);

    char comp[300], recon[300];
    snprintf(comp,300,"%s.huff",in);
    snprintf(recon,300,"reconstructed_%s",in);

    int ch;
    while(1){
        printf("\n1. Compress\n2. Decompress\n3. Show Size Comparison\n4. Exit\nChoice: ");
        scanf("%d",&ch);

        if(ch==1) compress_file(in, comp);
        else if(ch==2) decompress_file(comp, recon);
        else if(ch==3) show_table(in, comp, recon);
        else if(ch==4) break;
        else printf("Invalid choice.\n");
    }
    return 0;
}
