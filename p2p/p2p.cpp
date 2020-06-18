#include <mpi.h>
#include <iostream>
#include <cstdlib>
#include <sstream>
#include <string>
#include <fstream>

using namespace std;

int ortadeger(int dizi[])//dizimizi sıralamak için ayrı bir fonksiyon kullanıyoruz
{	
	for(int i=0;i<9;i++)
	{
		for(int j=0;j<9;j++) 
		{ 
			if(dizi[j]>dizi[j+1]) 
			{ 
				int temp=dizi[j]; 
				dizi[j]=dizi[j+1]; 
				dizi[j+1]=temp; 
			} 
		}
	}
	return dizi[4];//dizi geriye ortanca terimini döndürür
}
int main(int argc,char *argv[])
{	
	int size,myRank,boyut,kismiSatir,localHucre,localDeger,kismiSutun;//makinalar aras� ileti�im i�in gerekli olan de�i�kenler

	double startTime,endTime,elapsedTime;//zaman hesaplamas� i�in gerekli olan de�i�kenler

	unsigned char *kismiDizi,*girismatris,*cikismatris;//dinamik dizi pointerları 
	
	int s;//dosyadan okuduğumuz değerle geçici olarak bu değişkene atıyoruz
	int intdizi[9];//diziyi sıralamak için 9 boyutlu bir dizi tanımladık
	int satir = 0, sutun = 0,hucre;//dizi dinamik boyut değişkenleri tanımladık 
	
	MPI_Status status;//haberle�memiz i�in gerekli olan mpi rit�eli 

	MPI_Init(&argc,&argv);//mpi ba�lat�yoruz
	
	MPI_Comm_size(MPI_COMM_WORLD,&size);//toplam a�daki makina say�s�n� al�yoruz
	MPI_Comm_rank(MPI_COMM_WORLD,&myRank);//kendi numaras�n� bu de�i�kende tutacak her pc
	
	
	if(myRank==0)//e�er ana makina ise yap�lacak i�lemler
	{	
		string str = argv[1];//dosya adını almak için değişkene argümanımızı atıyoruz
		string nokta = ".";//uzantıyı almamak için spliti noktadan alıyoruz
		int adres = 0;//noktanın adresini tutmak için değişkenimiz
		string isim;//uzantısız isim için değişkenimiz
		adres = str.find(nokta);
		isim = str.substr(0, adres);//noktanın adresine kadar olan kısmı isim değişkenine atacak

		ifstream dosya;//dosyayı okuma modunda açıyoruz
		ofstream filtre;//dosyayı yazma modunda açıyoruz

		dosya.open(argv[1], ios::in);//dosyayı açıyoruz
		filtre.open(isim+"_filtered.txt", ios::out);//dosya ismini çıktı dosya yazdırıyoruz

		cout << "Dosya okunuyor..."<<endl;
		dosya >> satir;	//dizi boyutunu alıyoruz
		dosya >> sutun;
		
		if(((satir-2)%size)!=0)
		{
			cout<<"Goruntu matrisi esit dagitilamiyor.\n";
			MPI_Abort(MPI_COMM_WORLD,99);
		}
		
		girismatris=new unsigned char[satir*sutun];//dinamik matrislerimizi ayarlad�k
		cikismatris=new unsigned char[satir*sutun];
		
		for (int j = 0; j < satir; j++)//ardından okuma işlemi 2. satırdan devam ediyor ve tüm terimleri matrislerimizin içine dolduruyoruz.
		{
			for (int k = 0; k < sutun; k++)
			{
				dosya >> s;
				hucre=j*sutun+k;
				girismatris[hucre]=s;
				cikismatris[hucre] = girismatris[hucre];
			}
		}

		startTime=MPI_Wtime();//zamanlayıcı mpi.h kütüphanesi yardımıyla başlatıyoruz
		
		kismiSatir=(satir-2)/size;//sat�r baz�nda b�l�mlendirme ger�ekle�tirmek istedi�imiz i�in al�yoruz de�i�kenimize
		for(int i=1;i<size;i++)//di�er makinalara dizi i�in gerekli verileri g�nderdik 
		{
			MPI_Send(&kismiSatir,1,MPI_INT,i,25,MPI_COMM_WORLD);
			MPI_Send(&sutun,1,MPI_INT,i,25,MPI_COMM_WORLD);
		}
		for(int i=1;i<size;i++)//dizileri g�nderdik di�er makinalara
			MPI_Send(&girismatris[i*kismiSatir*sutun],((kismiSatir+2)*sutun),MPI_UNSIGNED_CHAR,i,25,MPI_COMM_WORLD);

		//ilk makina için işlemleri yaptırıyoruz
		int z=0;
		for (int f = 0; f < kismiSatir; f++, z = z - 3)//satırları gezer
		{
			int q = f + 3, p = z + 3;
			for (int z = 0; z < sutun - 2; z++, f = f - 3)//sütunları gezer 
			{
				int q = f + 3, p = z + 3, n = 0;
				while (f<q)//3x3 matris burda geçici dizimize atılıyor
				{
					while (z<p)
					{
						int hucre=f*sutun+z;
						int a = girismatris[hucre];
						intdizi[n]=a;
						n++;
						z++;
					}
					z = p - 3;
					f++;
				}
				int ortanca=ortadeger(intdizi);
				hucre=((f-2)*sutun)+(z+1);

				cikismatris[hucre]=ortanca;
			}
		}

		for(int i=1;i<size;i++)//ana makina olarak di�er makinalarda hesaplanm�� matris verisini ve h�cresini al�yoruz
		{	
			for(int j=0;j<kismiSatir*(sutun-2);j++)
			{	
				MPI_Recv(&localHucre,1,MPI_INT,i,25,MPI_COMM_WORLD,&status);
				MPI_Recv(&localDeger,1,MPI_INT,i,25,MPI_COMM_WORLD,&status);
				cikismatris[localHucre]=localDeger;
			}
		}

		endTime=MPI_Wtime();//zamanlay�c�y� bitiriyoruz
		elapsedTime=endTime-startTime;//ge�en zaman� buluyoruz
		cout<<endl<<"Toplam hesaplama icin gecen sure= "<<elapsedTime<<endl;

		for (int j = 0; j < satir; j++)//dosyaya yazdırma işlemini gerçekleştiriyoruz
		{
			for (int k = 0; k < sutun; k++)
			{
				int hucre=(j*sutun)+k;
				int a=cikismatris[hucre];
				filtre<<a;
				if(k<sutun-1)//sonda tab olmasını istemediğimden böyle bir koşul koydum
					filtre<<"\t";
			}
			if(j<satir-1)//son satırda boş bir satır olmaması için yine bu koşulu koydum
				filtre<<"\n";
		}
		delete [] girismatris;//dinamik dizileri siliyoruz
		delete [] cikismatris;
		dosya.close();
		filtre.close();
	}
	else//ana makina de�ilse yap�lacak i�lemler 
	{
		MPI_Recv(&kismiSatir,1,MPI_INT,0,25,MPI_COMM_WORLD,&status);//satir bilgisini al�yoruz
		MPI_Recv(&sutun,1,MPI_INT,0,25,MPI_COMM_WORLD,&status);//sutun bilgisini al�yoruz
		kismiSatir+=2;//hesaplanmas� gereken sat�rdan 2 sat�r fazlas� gerekli o y�zden dizi boyutunu ona g�re ald�k
		kismiDizi=new unsigned char[kismiSatir*sutun];//dinamik k�smi dizimiz olu�turuldu

		MPI_Recv(kismiDizi,kismiSatir*sutun,MPI_UNSIGNED_CHAR,0,25,MPI_COMM_WORLD,&status);//diziyi al�yoruz	
		
		int z=0;//hesaplama i�lemleri
		for (int f = 0; f < kismiSatir-2; f++, z = z - 3)
		{
			int q = f + 3, p = z + 3;
			for (int z = 0; z < sutun - 2; z++, f = f - 3)
			{
				int q = f + 3, p = z + 3, n = 0;
				while (f<q)
				{
					while (z<p)
					{
						int hucre=f*sutun+z;
						int a = kismiDizi[hucre];
						intdizi[n]=a;
						n++;
						z++;
					}
					z = p - 3;
					f++;
				}
				localDeger=ortadeger(intdizi);
				localHucre=(((f-2)*sutun)+(z+1))+((kismiSatir-2)*(sutun*myRank));
				//h�cre de�erini makinadan bu �ekilde bir denklemle do�ru bir �ekilde alabiliyoruz
				MPI_Send(&localHucre,1,MPI_INT,0,25,MPI_COMM_WORLD);//verileri ana makinaya g�nderiyoruz
				MPI_Send(&localDeger,1,MPI_INT,0,25,MPI_COMM_WORLD);
			}
		}	
		delete [] kismiDizi;//dinamik dizi siliniyor
	}

	MPI_Finalize();//mpi rutinini bitiriyoruz
	return 0;//THE END
}
