/*////////////////////////////
 * 2010/11/24  By paroid
 * Default Key: paroid
 * provide about 1/16[Wav.size] data space 
/////////////////////////////*/
#include <iostream>
#include <string.h>
#include <fstream>
#include <algorithm>
#include <time.h>

const int BLK=320;

using namespace std;

int rea(fstream &f,int co=4){
	unsigned char buff[4];
	f.read((char *)buff,co);
	int res=0;
	for(int i=co-1;i>=0;i--)
		res=(res<<8)|buff[i];
	return res;
}

void see(fstream &f,int n=0){
	char c;
	for(int i=0;i<n;i++){
		f.get(c);
		cout<<c;
	}
	return;
}


void itc(int i,char *c){
	c[0]=char(i & 0xff);
	c[1]=char((i & 0xff00)>>8);
	c[2]=char((i & 0xff0000)>>16);
	c[3]=char((i & 0xff000000)>>24);
}

void cti(char *c,int &i){
	i=c[0] & 0xff;
	i|=(c[1]<<8) & 0xff00;
	i|=(c[2]<<16) & 0xff0000;
	i|=(c[3]<<24) & 0xff000000;
}

void emb(fstream &fi,fstream &fo,int n,char *d){
	char bu[16];
	for(int i=0;i<n;i++){
		fi.read(bu,16);
		for(int j=0;j<8;j++){
			bu[2*j]=(d[i]&0x80)?bu[2*j]|1:bu[2*j]&0xfe;
			d[i]<<=1;
			fo.put(bu[2*j]);
			fo.put(bu[2*j+1]);
		}
	}
}

void ext(fstream &f,int n,char *d){
	char bu[2];
	for(int i=0;i<n;i++){
		d[i]=0;
		for(int j=0;j<8;j++){
			f.read(bu,2);
			d[i]=(d[i]<<1)|(bu[0]&1);
		}
	}
}

void RC4(int n,char *in,char *out){
	char key[]="paroid";
	int l=4,ss[256];
	for(int i=0;i<256;i++)
		ss[i]=i;
	for(int j,i=0;i<l;i++){
		j=(i+ss[i]+key[i])%256;
		swap(ss[i],ss[j]);
	}
	for(int i=0,j=0,k=0;k<n;k++){
		i=(i+1)%256;
		j=(j+ss[i])%256;
		swap(ss[i],ss[j]);
		int t=(ss[i]+ss[j])%256;
		out[k]=ss[t]^in[k];
	}
}

int main(){
	bool bol=true;
	char fn[32],tc,buffer[(BLK>320)?BLK:320],tmp[(BLK>320)?BLK:320];
	int mark,ct=0,dl,fl;
	time_t time;
	while(bol){
		fstream fi,fo,fm;
		system("color 71");
		system("cls");
		do{
			cout<<(fi?"":"File Open Error!")<<"Input Source Filename"<<endl;
			cin>>fn;
			fi.open(fn,ios::in|ios::binary);
		}while(!fi.is_open());

		system("cls");
		cout<<"RIFF WAVE  chunk============"<<endl;
		cout<<"ID:             |";	see(fi,4);	cout<<endl;
		cout<<"FileSize:       |"<<rea(fi)<<endl;
		cout<<"RiffType:       |";	see(fi,4);	cout<<endl;
		ct+=12;
		cout<<"Fomat  Chunk================="<<endl;
		cout<<"ID:             |";	see(fi,4);	cout<<endl;
		cout<<"WithExtrainfo?: |"<<(mark=rea(fi))<<"( 16<No> 18<Yes> )"<<endl;
		cout<<"FormatTag:      |"<<rea(fi,2)<<endl;
		cout<<"Channel:        |"<<rea(fi,2)<<endl;
		cout<<"SamplesPerSec:  |"<<rea(fi)<<endl;
		cout<<"AvgBytesPerSec: |"<<rea(fi)<<endl;
		cout<<"BlockAlign:     |"<<rea(fi,2)<<endl;
		cout<<"BitsPerSample:  |"<<rea(fi,2)<<endl;
		cout<<"ExtrInfo:       |";
		ct+=24;
		if(mark==18){
			see(fi,2);
			ct+=2;
		}
		else	cout<<"No";
		cout<<endl;

		fi.get(tc);
		if(tc=='f'){
			cout<<"Fact Chunk==============="<<endl;
			cout<<"ID:          | f";	see(fi,3); cout<<endl;
			cout<<"Size:        |"<<rea(fi)<<endl;
			cout<<"Data:        |";		see(fi,4);	cout<<endl;
			ct+=12;
		}
		cout<<"Data Chunnk =================="<<endl;
		cout<<"ID:            |";
		if(tc=='f')		see(fi,4);
		else{
			cout<<tc;
			see(fi,3);
		}
		cout<<endl;
		cout<<"Size:          |"<<(dl=rea(fi))<<endl;
		cout<<"------------------------------------------"<<endl;
		ct+=8;

		cout<<"\n\n<0> Embed <1> Extract\n"<<endl;
		cin>>bol;

		if(bol){
			fi.read(buffer,4);
			RC4(4,buffer,tmp);
			ext(fi,4,buffer);
			tmp[4]=buffer[4]='\0';
			if(strcmp(buffer,tmp))
				cout<<"No WaterMark Inside !"<<endl;
			else{
				ext(fi,32,buffer);
				RC4(32,buffer,fn);
				fo.open(fn,ios::out|ios::binary);
				ext(fi,4,tmp);
				RC4(4,tmp,buffer);
				cti(buffer,fl);
				cout<<"Embed File:  "<<fn<<"Length: "<<(double(fl)/1024.0)<<" KB"<<endl;
				cout<<"Extracting ..."<<endl;
				time=clock();
				for(int i=0,block=fl/BLK;i<block;i++){
					ext(fi,BLK,tmp);
					RC4(BLK,tmp,buffer);
					fo.write(buffer,fl%BLK);
					cout<<"Time : "<<double(clock()-time)/1000.0<<" s"<<endl;					
				}
			}
		}
		else{
			fn[strlen(fn)-4]='\0';
			strcat(fn,"@.wav");
			fo.open(fn,ios::out|ios::binary);
			do{
				cout<<(fn?"":"File Open Error !")<<"Input WaterMark Filename (Max: "<<double(dl/16-40)/1024.0<<" KB )"<<endl;
				cin>>fn;
				fm.open(fn,ios::in|ios::binary);
				if(fm.is_open()){
					fm.seekg(ios::beg);
					fl=fm.tellg();
					fm.seekg(0,ios::end);
					fl=int(fm.tellg())-fl;
					if(fl>dl/16-40){
						cout<<"File too Big !"<<endl;
						fm.close();
					}
				}
			}while(!fm.is_open());
			cout<<"Embeding ... "<<endl;
			time=clock();
			fi.seekg(ios::beg);
			fm.seekg(ios::beg);
			fi.read(buffer,ct);
			fo.write(buffer,ct);
			fi.read(buffer,4);
			fo.write(buffer,4);
			RC4(4,buffer,tmp);
			emb(fi,fo,4,tmp);
			char rname[32]="@";
			strcat(rname,fn);
			strcpy(fn,rname);
			RC4(32,fn,tmp);
			emb(fi,fo,32,tmp);
			itc(fl,tmp);
			RC4(4,tmp,buffer);
			emb(fi,fo,4,buffer);
			for(int i=0,block=fl/BLK;i<block;i++){
				fm.read(buffer,BLK);
				RC4(BLK,buffer,tmp);
				emb(fi,fo,BLK,tmp);
			}
			fm.read(buffer,fl%BLK);
			RC4(fl%BLK,buffer,tmp);
			emb(fi,fo,fl%BLK,tmp);
			while(fi.read(buffer,BLK),fl=fi.gcount())
				fo.write(buffer,fl);
			fm.close();
			cout<<"Time: "<<double(clock()-time)/1000.0<<" s"<<endl;
		}
		fi.close();
		fo.close();
		cout<<"Done !"<<endl;
		cout<<"Continue? <1/0>"<<endl;
		cin>>bol;

	}
	return 0;
}




















