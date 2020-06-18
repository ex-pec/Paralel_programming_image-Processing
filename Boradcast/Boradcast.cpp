#include <mpi.h>
#include <iostream>
#include <cstdlib>
#include <sstream>
#include <string>
#include <fstream>

using namespace std;

int ortadeger(int dizi[])//dizimizi s�ralamak i�in ayr� bir fonksiyon kullan�yoruz
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
	return dizi[4];//dizi geriye ortanca terimini d�nd�r�r
}
int main(int argc,char *argv[])
{	
	int size,myRank,kismiSatir;//makinalar aras� ileti�im i�in gerekli olan de�i�kenler

	double startTime,endTime,elapsedTime;//zaman hesaplamas� i�in gerekli olan de�i�kenler

	unsigned char *kismiDizi,*kismiDizi2,*kismiDiziust,*kismiDizialt,*girismatris,*cikismatris;//dinamik dizi pointerlar� 
	
	int s;//dosyadan okudu�umuz de�erle ge�ici olarak bu de�i�kene at�yoruz
	int intdizi[9];//diziyi s�ralamak i�in 9 boyutlu bir dizi tan�mlad�k
	int satir = 0, sutun = 0,hucre;//dizi dinamik boyut de�i�kenleri tan�mlad�k 
	
	ifstream dosya;//dosyay� okuma modunda a��yoruz
	ofstream filtre;//dosyay� yazma modunda a��yoruz
	
	MPI_Init(&argc,&argv);//mpi ba�lat�yoruz
	
	MPI_Comm_size(MPI_COMM_WORLD,&size);//toplam a�daki makina say�s�n� al�yoruz
	MPI_Comm_rank(MPI_COMM_WORLD,&myRank);//kendi numaras�n� bu de�i�kende tutacak her pc
	
	
	if(myRank==0)//e�er ana makina ise yap�lacak i�lemler
	{
		dosya.open(argv[1], ios::in);//dosyay� a��yoruz

		cout << "Dosya okunuyor..."<<endl;
		dosya >> satir;	//dizi boyutunu al�yoruz
		dosya >> sutun;
		
		if(((satir-2)%size)!=0)//e�it da��t�lam�yorsa hata verir
		{
			cout<<"Goruntu matrisi esit dagitilamiyor.\n";
			MPI_Abort(MPI_COMM_WORLD,99);
		}
		
		girismatris=new unsigned char[satir*sutun];//dinamik matrislerimizi ayarlad�k
		cikismatris=new unsigned char[satir*sutun];
		
		kismiSatir=(satir-2)/size;//sat�r baz�nda b�l�mlendirme ger�ekle�tirmek istedi�imiz i�in al�yoruz de�i�kenimize
		
		kismiDizialt=new unsigned char[size*sutun];//alt ve �st sat�rlar� g�ndermek i�in olu�turuldu
		kismiDiziust=new unsigned char[size*sutun];
		for (int j = 0; j < satir; j++)//ard�ndan okuma i�lemi 2. sat�rdan devam ediyor ve t�m terimleri matrislerimizin i�ine dolduruyoruz.
		{
			for (int k = 0; k < sutun; k++)
			{
				dosya >> s;
				hucre=j*sutun+k;
				girismatris[hucre]=s;
				cikismatris[hucre]=s;
			}
		}
		startTime=MPI_Wtime();//zamanlay�c� mpi.h k�t�phanesi yard�m�yla ba�lat�yoruz
		for (int i = 0,k=0,l=kismiSatir+1; i < size; i++,k+=kismiSatir,l+=kismiSatir)
		{//i�lenecek verinin bir alt ve bir �st sat�rlar�n� al�yoruz dizimize
			for (int j = 0; j < sutun; j++)
			{
				hucre=i*sutun+j;
				int a=k*sutun+j;
				int b=l*sutun+j;
				kismiDiziust[hucre] = girismatris[a];
				kismiDizialt[hucre] = girismatris[b];
			}
		}
	}
	
	MPI_Bcast(&kismiSatir,1,MPI_INT,0,MPI_COMM_WORLD);//t�m cihazlara sat�r ve sutun say�lar�n� g�nderiyoruz
	MPI_Bcast(&sutun,1,MPI_INT,0,MPI_COMM_WORLD);
	kismiSatir+=2;//alt ve �st sat�rlar i�in iki sat�r art�r�yoruz satir de�i�kenimizi
	
	kismiDizi=new unsigned char[kismiSatir*sutun];//di�er cihazlarda da dinamik matrislerimizi ay�rd�k
	kismiDizi2=new unsigned char[kismiSatir*sutun];
	//scatter ile diziyi e�it bi �ekilde da��t�yoruz �nce �st sat�r� sonra i�lenecek veri sat�r�n� sonra bi alt sat�r�n�
	MPI_Scatter(kismiDiziust,sutun,MPI_UNSIGNED_CHAR,&kismiDizi[0],sutun,MPI_UNSIGNED_CHAR,0,MPI_COMM_WORLD);
	MPI_Scatter(&girismatris[sutun],((kismiSatir-2)*sutun),MPI_UNSIGNED_CHAR,&kismiDizi[sutun],((kismiSatir-2)*sutun),MPI_UNSIGNED_CHAR,0,MPI_COMM_WORLD);
	MPI_Scatter(kismiDizialt,sutun,MPI_UNSIGNED_CHAR,&kismiDizi[((kismiSatir-1)*sutun)],sutun,MPI_UNSIGNED_CHAR,0,MPI_COMM_WORLD);
	//makinalarda i�lenmeye verilerin hesaplanmas�ndan kurtulmak i�in ��kt� matrisine girdi matrisini kopyal�yoruz
	for (int i = 0; i < kismiSatir; i++)
	{
		for (int j = 0; j < sutun; j++)
		{
			hucre=i*sutun+j;
			kismiDizi2[hucre]=kismiDizi[hucre];
		}
	}
	
	int z=0;//hesaplama i�lemi ger�ekle�ir
	for (int f = 0; f < kismiSatir-2; f++, z = z - 3)//sat�rlar� gezer
	{
		int q = f + 3, p = z + 3;
		for (int z = 0; z < sutun - 2; z++, f = f - 3)//s�tunlar� gezer 
		{
			int q = f + 3, p = z + 3, n = 0;
			while (f<q)//3x3 matris burda ge�ici dizimize at�l�yor
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
			int ortanca=ortadeger(intdizi);
			hucre=((f-2)*sutun)+(z+1);
			kismiDizi2[hucre]=ortanca;//g�nderilecek matrise verileri s�ras�yla kaydediyoruz
		}
	}
	//t�m verilerin i�lenmi� k�sm� �st ve alt sat�rdan ba��ms�z olan k�s�mlar oralar� gather yap�s� ile ana matrisimize toplad�k
	MPI_Gather(&kismiDizi2[sutun],((kismiSatir-2)*sutun),MPI_UNSIGNED_CHAR,&cikismatris[sutun],((kismiSatir-2)*sutun),MPI_UNSIGNED_CHAR,0,MPI_COMM_WORLD);

	if(myRank==0)
	{
		endTime=MPI_Wtime();//zamanlay�c�y� bitiriyoruz
		elapsedTime=endTime-startTime;//ge�en zaman� buluyoruz
		cout<<endl<<"Toplam hesaplama icin gecen sure= "<<elapsedTime<<endl;
	
		string str = argv[1];//dosya ad�n� almak i�in de�i�kene arg�man�m�z� at�yoruz
		string nokta = ".";//uzant�y� almamak i�in spliti noktadan al�yoruz
		int adres = 0;//noktan�n adresini tutmak i�in de�i�kenimiz
		string isim;//uzant�s�z isim i�in de�i�kenimiz
		adres = str.find(nokta);
		isim = str.substr(0, adres);//noktan�n adresine kadar olan k�sm� isim de�i�kenine atacak

		filtre.open(isim+"_filtered.txt", ios::out);//dosya ismini ��kt� dosya yazd�r�yoruz

		for (int j = 0; j < satir; j++)//dosyaya yazd�rma i�lemini ger�ekle�tiriyoruz
		{
			for (int k = 0; k < sutun; k++)
			{
				int hucre=(j*sutun)+k;
				int a=cikismatris[hucre];
				filtre<<a;
				if(k<sutun-1)//sonda tab olmas�n� istemedi�imden b�yle bir ko�ul koydum
					filtre<<"\t";
			}
			if(j<satir-1)//son sat�rda bo� bir sat�r olmamas� i�in yine bu ko�ulu koydum
				filtre<<"\n";
		}
		delete [] girismatris;//dinamik dizileri siliyoruz
		delete [] cikismatris;
		delete [] kismiDizialt;
		delete [] kismiDiziust;

		dosya.close();//dosyalar� kapatt�k
		filtre.close();

	}
	delete [] kismiDizi;//dinamik dizi siliniyor
	delete [] kismiDizi2;
	MPI_Finalize();//mpi rutinini bitiriyoruz

	return 0;//THE END
}
