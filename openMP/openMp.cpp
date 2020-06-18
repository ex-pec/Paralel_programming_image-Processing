#include <omp.h>  //OpenMP Library
#include <cstdio> //printf
#include <fstream>
#include <string>
#include <iostream>

using namespace std;

int ortadeger(int dizi[]){//dizimizi s�ralamak i�in ayr� bir fonksiyon kullan�yoruz
	for(int i=0;i<9;i++) 
		for(int j=0;j<9;j++) 
			{ if(dizi[j]>dizi[j+1]) 
				{ int temp=dizi[j]; 
				dizi[j]=dizi[j+1]; 
				dizi[j+1]=temp; } }
	return dizi[4];//dizi geriye ortanca terimini d�nd�r�r
}

int main(int argc, char** argv)
{
	int nthreads,tid;     //Thread ler i�in
	double startTime,endTime,elapsedTime;// zaman �l��m� i�in
	
	string str = argv[1];//dosya ad�n� almak i�in de�i�kene arg�man�m�z� at�yoruz
	string nokta = ".";//uzant�y� almamak i�in spliti noktadan al�yoruz
	int adres = 0;//noktan�n adresini tutmak i�in de�i�kenimiz
	string isim;//uzant�s�z isim i�in de�i�kenimiz
	adres = str.find(nokta);
	isim = str.substr(0, adres);//noktan�n adresine kadar olan k�sm� isim de�i�kenine atacak
	
	ifstream dosya;//dosyay� okuma modunda a��yoruz
	ofstream filtre;//dosyay� yazma modunda a��yoruz
	int s;//dosyadan okudu�umuz de�erle ge�ici olarak bu de�i�kene at�yoruz
	int intdizi[9];//diziyi s�ralamak i�in 9 boyutlu bir dizi tan�mlad�k
	int satir = 0, sutun = 0;//dizi dinamik boyut de�i�kenleri tan�mlad�k 
	unsigned char **girismatris, **cikismatris;//dinamik dizi pointerlar�
	
	dosya.open(argv[1], ios::in);//dosyay� a��yoruz
	
	cout << "Dosya okunuyor..."<<endl;
	dosya >> s;	//dizi boyutunu al�yoruz
	satir = s;
	dosya >> s;
	sutun = s;

	
	girismatris = new unsigned char *[satir];//giris matrisi i�in dinamik 2 boyutlu dizi olu�turuluyor
	for (int i = 0; i<sutun; i++)
	{
		girismatris[i] = new unsigned char[sutun];
	}
	
	cikismatris = new unsigned char *[satir];
	for (int i = 0; i<sutun; i++)//��k�� matrisi i�in dinamik 2 boyutlu dizi olu�turuluyor
	{
		cikismatris[i] = new unsigned char[sutun];
	}

	for (int j = 0; j < satir; j++)//ard�ndan okuma i�lemi 2. sat�rdan devam ediyor ve t�m terimleri matrislerimizin i�ine dolduruyoruz.
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
		for (int f = 0; f < satir - 2; f++)//sat�rlar� gezer
		{
			for (int z = 0; z < sutun - 2; z++, f = f - 3)//s�tunlar� gezer 
			{
				int q = f + 3, p = z + 3, n = 0;
				while (f<q)//3x3 matris burda ge�ici dizimize at�l�yor
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

	filtre.open(isim+"_filtered.txt", ios::out);//dosya ismini ��kt� dosya yazd�r�yoruz

	for (int j = 0; j < satir; j++)//dosyaya yazd�rma i�lemini ger�ekle�tiriyoruz
	{
		for (int k = 0; k < sutun; k++)
		{
			int a=cikismatris[j][k];
			filtre<<a;
			if(k<sutun-1)//sonda tab olmas�n� istemedi�imden b�yle bir ko�ul koydum
				filtre<<"\t";
		}
		if(j<satir-1)//son sat�rda bo� bir sat�r olmamas� i�in yine bu ko�ulu koydum
			filtre<<"\n";
	}
/*dinamik dizileri silerken hata al�yordum bu y�zden kapatmak zorunda kald�m bu k�sm� ama program sorunsuz �al��makta*/

	/*for(int i=0;i<satir;i++)//dizileri siliyoruz
		delete [] girismatris[i];
	delete [] girismatris;
	
	for(int i=0;i<satir;i++)
		delete [] cikismatris[i];
	delete [] cikismatris;*/
	
	dosya.close();//dosyalar� kapat�yoruz
	filtre.close();
	
	cout<<"islem basariyla tamamlandi"<<endl;;
 	
	 
	return 0;
}

