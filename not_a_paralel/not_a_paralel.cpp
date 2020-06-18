#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include <cstdlib>
#include <sys/time.h>

using namespace std;

int ortadeger(int dizi[]){//dizimizi sıralamak için ayrı bir fonksiyon kullanıyoruz
	for(int i=0;i<9;i++) 
		for(int j=0;j<9;j++) 
			{ if(dizi[j]>dizi[j+1]) 
				{ int temp=dizi[j]; 
				dizi[j]=dizi[j+1]; 
				dizi[j+1]=temp; } }
	return dizi[4];//dizi geriye ortanca terimini döndürür
}

int main(int argc, char** argv) {
	string str = argv[1];//dosya adını almak için değişkene argümanımızı atıyoruz
	string nokta = ".";//uzantıyı almamak için spliti noktadan alıyoruz
	int adres = 0;//noktanın adresini tutmak için değişkenimiz
	string isim;//uzantısız isim için değişkenimiz
	adres = str.find(nokta);
	isim = str.substr(0, adres);//noktanın adresine kadar olan kısmı isim değişkenine atacak
	
	ifstream dosya;//dosyayı okuma modunda açıyoruz
	ofstream filtre;//dosyayı yazma modunda açıyoruz
	int s;//dosyadan okuduğumuz değerle geçici olarak bu değişkene atıyoruz
	int intdizi[9];//diziyi sıralamak için 9 boyutlu bir dizi tanımladık
	int satir = 0, sutun = 0, n = 0;//dizi dinamik boyut değişkenleri tanımladık 
	unsigned char **girismatris, **cikismatris;//dinamik dizi pointerları 
	
	dosya.open(argv[1], ios::in);//dosyayı açıyoruz
	filtre.open(isim+"_filtered.txt", ios::out);//dosya ismini çıktı dosya yazdırıyoruz


	dosya >> s;	//dizi boyutunu alıyoruz
	satir = s;
	dosya >> s;
	sutun = s;
	cout << "Dosya okunuyor..."<<endl;
	
	girismatris = new unsigned char *[satir];//giris matrisi için dinamik 2 boyutlu dizi oluşturuluyor
	for (int i = 0; i<sutun; i++)
	{
		girismatris[i] = new unsigned char[sutun];
	}
	
	cikismatris = new unsigned char *[satir];
	for (int i = 0; i<sutun; i++)//çıkış matrisi için dinamik 2 boyutlu dizi oluşturuluyor
	{
		cikismatris[i] = new unsigned char[sutun];
	}

	for (int j = 0; j < satir; j++)//ardından okuma işlemi 2. satırdan devam ediyor ve tüm terimleri matrislerimizin içine dolduruyoruz.
	{
		for (int k = 0; k < sutun; k++)
		{
			dosya >> s;
			girismatris[j][k]=s;
			cikismatris[j][k] = girismatris[j][k];
		}
		
	}
	struct timeval currentTime;
	double startTime,endTime,elapsedTime;
	gettimeofday(&currentTime, NULL); //
	startTime=currentTime.tv_sec+(currentTime.tv_usec/1000000.0);	

	int f=0,z=0;//içerde kullanacağımız için ön değerli değişken kullanıyoruz
	for (int f = 0; f < satir - 2; f++, z = z - 3)//satırları gezer
	{
		int q = f + 3, p = z + 3;
		for (int z = 0; z < sutun - 2; z++, f = f - 3)//sütunları gezer 
		{
			int q = f + 3, p = z + 3, n = 0;
			while (f<q)//3x3 matris burda geçici dizimize atılıyor
			{
				while (z<p)
				{
					int a = girismatris[f][z];
					intdizi[n]=a;
					n++;
					z++;
				}
				z = p - 3;
				f++;
			}
			int orta=ortadeger(intdizi);
			cikismatris[f-2][z+1]=orta;
		}
	}
	
	gettimeofday(&currentTime, NULL); //Secmnds frmm the epmch tme
	endTime=currentTime.tv_sec+(currentTime.tv_usec/1000000.0);
	// Calculate the elapsed tme in secmnds
	elapsedTime = endTime-startTime;
	cout << "Total Time Taken (saniye cinsinden): " << elapsedTime<< endl;

	for (int j = 0; j < satir; j++)//dosyaya yazdırma işlemini gerçekleştiriyoruz
	{
		for (int k = 0; k < sutun; k++)
		{
			int a=cikismatris[j][k];
			filtre<<a;
			if(k<sutun-1)//sonda tab olmasını istemediğimden böyle bir koşul koydum
				filtre<<"\t";
		}
		if(j<satir-1)//son satırda boş bir satır olmaması için yine bu koşulu koydum
			filtre<<"\n";
	}

	for(int i=0;i<satir;i++)//dizileri siliyoruz
		delete [] girismatris[i];
	delete [] girismatris;
	for(int i=0;i<satir;i++)
		delete [] cikismatris[i];
	delete [] cikismatris;
	dosya.close();//dosyaları kapatıyoruz
	filtre.close();
	cout<<"islem basarıyla tamamlandı";
	return 0;
}

