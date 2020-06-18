#include <omp.h>  //OpenMP Library
#include <cstdio> //printf
#include <fstream>
#include <string>
#include <iostream>

using namespace std;

int ortadeger(int dizi[]){//dizimizi sýralamak için ayrý bir fonksiyon kullanýyoruz
	for(int i=0;i<9;i++) 
		for(int j=0;j<9;j++) 
			{ if(dizi[j]>dizi[j+1]) 
				{ int temp=dizi[j]; 
				dizi[j]=dizi[j+1]; 
				dizi[j+1]=temp; } }
	return dizi[4];//dizi geriye ortanca terimini döndürür
}

int main(int argc, char** argv)
{
	int nthreads,tid;     //Thread ler için
	double startTime,endTime,elapsedTime;// zaman ölçümü için
	
	string str = argv[1];//dosya adýný almak için deðiþkene argümanýmýzý atýyoruz
	string nokta = ".";//uzantýyý almamak için spliti noktadan alýyoruz
	int adres = 0;//noktanýn adresini tutmak için deðiþkenimiz
	string isim;//uzantýsýz isim için deðiþkenimiz
	adres = str.find(nokta);
	isim = str.substr(0, adres);//noktanýn adresine kadar olan kýsmý isim deðiþkenine atacak
	
	ifstream dosya;//dosyayý okuma modunda açýyoruz
	ofstream filtre;//dosyayý yazma modunda açýyoruz
	int s;//dosyadan okuduðumuz deðerle geçici olarak bu deðiþkene atýyoruz
	int intdizi[9];//diziyi sýralamak için 9 boyutlu bir dizi tanýmladýk
	int satir = 0, sutun = 0;//dizi dinamik boyut deðiþkenleri tanýmladýk 
	unsigned char **girismatris, **cikismatris;//dinamik dizi pointerlarý
	
	dosya.open(argv[1], ios::in);//dosyayý açýyoruz
	
	cout << "Dosya okunuyor..."<<endl;
	dosya >> s;	//dizi boyutunu alýyoruz
	satir = s;
	dosya >> s;
	sutun = s;

	
	girismatris = new unsigned char *[satir];//giris matrisi için dinamik 2 boyutlu dizi oluþturuluyor
	for (int i = 0; i<sutun; i++)
	{
		girismatris[i] = new unsigned char[sutun];
	}
	
	cikismatris = new unsigned char *[satir];
	for (int i = 0; i<sutun; i++)//çýkýþ matrisi için dinamik 2 boyutlu dizi oluþturuluyor
	{
		cikismatris[i] = new unsigned char[sutun];
	}

	for (int j = 0; j < satir; j++)//ardýndan okuma iþlemi 2. satýrdan devam ediyor ve tüm terimleri matrislerimizin içine dolduruyoruz.
	{
		for (int k = 0; k < sutun; k++)
		{
			dosya >> s;
			girismatris[j][k]=s;
			cikismatris[j][k] = girismatris[j][k];
		}
		
	}
	
	string thr=argv[2];
	int thread=atoi(thr.c_str());
	omp_set_num_threads(thread);
	
	#pragma omp parallel shared(girismatris,cikismatris,satir,sutun) private(nthreads,tid,startTime,endTime,elapsedTime,intdizi)
 	{
		/* Obtain thread number */
 		tid=omp_get_thread_num();
 		
		if(tid==0)
 		{
 			startTime=omp_get_wtime();
 		}
 		
		#pragma omp for
		for (int f = 0; f < satir - 2; f++)//satýrlarý gezer
		{
			for (int z = 0; z < sutun - 2; z++, f = f - 3)//sütunlarý gezer 
			{
				int q = f + 3, p = z + 3, n = 0;
				while (f<q)//3x3 matris burda geçici dizimize atýlýyor
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
		
		if(tid==0)
 		{
 			endTime=omp_get_wtime();
 			elapsedTime=endTime-startTime;
 			cout<<"gecen sure= "<<elapsedTime<<endl;
 		}
 	}

	filtre.open(isim+"_filtered.txt", ios::out);//dosya ismini çýktý dosya yazdýrýyoruz

	for (int j = 0; j < satir; j++)//dosyaya yazdýrma iþlemini gerçekleþtiriyoruz
	{
		for (int k = 0; k < sutun; k++)
		{
			int a=cikismatris[j][k];
			filtre<<a;
			if(k<sutun-1)//sonda tab olmasýný istemediðimden böyle bir koþul koydum
				filtre<<"\t";
		}
		if(j<satir-1)//son satýrda boþ bir satýr olmamasý için yine bu koþulu koydum
			filtre<<"\n";
	}
/*dinamik dizileri silerken hata alýyordum bu yüzden kapatmak zorunda kaldým bu kýsmý ama program sorunsuz çalýþmakta*/

	/*for(int i=0;i<satir;i++)//dizileri siliyoruz
		delete [] girismatris[i];
	delete [] girismatris;
	
	for(int i=0;i<satir;i++)
		delete [] cikismatris[i];
	delete [] cikismatris;*/
	
	dosya.close();//dosyalarý kapatýyoruz
	filtre.close();
	
	cout<<"islem basariyla tamamlandi"<<endl;;
 	
	 
	return 0;
}

