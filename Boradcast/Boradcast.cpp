#include <mpi.h>
#include <iostream>
#include <cstdlib>
#include <sstream>
#include <string>
#include <fstream>

using namespace std;

int ortadeger(int dizi[])//dizimizi sýralamak için ayrý bir fonksiyon kullanýyoruz
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
	int size,myRank,kismiSatir;//makinalar arasý iletiþim için gerekli olan deðiþkenler

	double startTime,endTime,elapsedTime;//zaman hesaplamasý için gerekli olan deðiþkenler

	unsigned char *kismiDizi,*kismiDizi2,*kismiDiziust,*kismiDizialt,*girismatris,*cikismatris;//dinamik dizi pointerlarý 
	
	int s;//dosyadan okuduðumuz deðerle geçici olarak bu deðiþkene atýyoruz
	int intdizi[9];//diziyi sýralamak için 9 boyutlu bir dizi tanýmladýk
	int satir = 0, sutun = 0,hucre;//dizi dinamik boyut deðiþkenleri tanýmladýk 
	
	ifstream dosya;//dosyayý okuma modunda açýyoruz
	ofstream filtre;//dosyayý yazma modunda açýyoruz
	
	MPI_Init(&argc,&argv);//mpi baþlatýyoruz
	
	MPI_Comm_size(MPI_COMM_WORLD,&size);//toplam aðdaki makina sayýsýný alýyoruz
	MPI_Comm_rank(MPI_COMM_WORLD,&myRank);//kendi numarasýný bu deðiþkende tutacak her pc
	
	
	if(myRank==0)//eðer ana makina ise yapýlacak iþlemler
	{
		dosya.open(argv[1], ios::in);//dosyayý açýyoruz

		cout << "Dosya okunuyor..."<<endl;
		dosya >> satir;	//dizi boyutunu alýyoruz
		dosya >> sutun;
		
		if(((satir-2)%size)!=0)//eþit daðýtýlamýyorsa hata verir
		{
			cout<<"Goruntu matrisi esit dagitilamiyor.\n";
			MPI_Abort(MPI_COMM_WORLD,99);
		}
		
		girismatris=new unsigned char[satir*sutun];//dinamik matrislerimizi ayarladýk
		cikismatris=new unsigned char[satir*sutun];
		
		kismiSatir=(satir-2)/size;//satýr bazýnda bölümlendirme gerçekleþtirmek istediðimiz için alýyoruz deðiþkenimize
		
		kismiDizialt=new unsigned char[size*sutun];//alt ve üst satýrlarý göndermek için oluþturuldu
		kismiDiziust=new unsigned char[size*sutun];
		for (int j = 0; j < satir; j++)//ardýndan okuma iþlemi 2. satýrdan devam ediyor ve tüm terimleri matrislerimizin içine dolduruyoruz.
		{
			for (int k = 0; k < sutun; k++)
			{
				dosya >> s;
				hucre=j*sutun+k;
				girismatris[hucre]=s;
				cikismatris[hucre]=s;
			}
		}
		startTime=MPI_Wtime();//zamanlayýcý mpi.h kütüphanesi yardýmýyla baþlatýyoruz
		for (int i = 0,k=0,l=kismiSatir+1; i < size; i++,k+=kismiSatir,l+=kismiSatir)
		{//iþlenecek verinin bir alt ve bir üst satýrlarýný alýyoruz dizimize
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
	
	MPI_Bcast(&kismiSatir,1,MPI_INT,0,MPI_COMM_WORLD);//tüm cihazlara satýr ve sutun sayýlarýný gönderiyoruz
	MPI_Bcast(&sutun,1,MPI_INT,0,MPI_COMM_WORLD);
	kismiSatir+=2;//alt ve üst satýrlar için iki satýr artýrýyoruz satir deðiþkenimizi
	
	kismiDizi=new unsigned char[kismiSatir*sutun];//diðer cihazlarda da dinamik matrislerimizi ayýrdýk
	kismiDizi2=new unsigned char[kismiSatir*sutun];
	//scatter ile diziyi eþit bi þekilde daðýtýyoruz önce üst satýrý sonra iþlenecek veri satýrýný sonra bi alt satýrýný
	MPI_Scatter(kismiDiziust,sutun,MPI_UNSIGNED_CHAR,&kismiDizi[0],sutun,MPI_UNSIGNED_CHAR,0,MPI_COMM_WORLD);
	MPI_Scatter(&girismatris[sutun],((kismiSatir-2)*sutun),MPI_UNSIGNED_CHAR,&kismiDizi[sutun],((kismiSatir-2)*sutun),MPI_UNSIGNED_CHAR,0,MPI_COMM_WORLD);
	MPI_Scatter(kismiDizialt,sutun,MPI_UNSIGNED_CHAR,&kismiDizi[((kismiSatir-1)*sutun)],sutun,MPI_UNSIGNED_CHAR,0,MPI_COMM_WORLD);
	//makinalarda iþlenmeye verilerin hesaplanmasýndan kurtulmak için çýktý matrisine girdi matrisini kopyalýyoruz
	for (int i = 0; i < kismiSatir; i++)
	{
		for (int j = 0; j < sutun; j++)
		{
			hucre=i*sutun+j;
			kismiDizi2[hucre]=kismiDizi[hucre];
		}
	}
	
	int z=0;//hesaplama iþlemi gerçekleþir
	for (int f = 0; f < kismiSatir-2; f++, z = z - 3)//satýrlarý gezer
	{
		int q = f + 3, p = z + 3;
		for (int z = 0; z < sutun - 2; z++, f = f - 3)//sütunlarý gezer 
		{
			int q = f + 3, p = z + 3, n = 0;
			while (f<q)//3x3 matris burda geçici dizimize atýlýyor
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
			kismiDizi2[hucre]=ortanca;//gönderilecek matrise verileri sýrasýyla kaydediyoruz
		}
	}
	//tüm verilerin iþlenmiþ kýsmý üst ve alt satýrdan baðýmsýz olan kýsýmlar oralarý gather yapýsý ile ana matrisimize topladýk
	MPI_Gather(&kismiDizi2[sutun],((kismiSatir-2)*sutun),MPI_UNSIGNED_CHAR,&cikismatris[sutun],((kismiSatir-2)*sutun),MPI_UNSIGNED_CHAR,0,MPI_COMM_WORLD);

	if(myRank==0)
	{
		endTime=MPI_Wtime();//zamanlayýcýyý bitiriyoruz
		elapsedTime=endTime-startTime;//geçen zamaný buluyoruz
		cout<<endl<<"Toplam hesaplama icin gecen sure= "<<elapsedTime<<endl;
	
		string str = argv[1];//dosya adýný almak için deðiþkene argümanýmýzý atýyoruz
		string nokta = ".";//uzantýyý almamak için spliti noktadan alýyoruz
		int adres = 0;//noktanýn adresini tutmak için deðiþkenimiz
		string isim;//uzantýsýz isim için deðiþkenimiz
		adres = str.find(nokta);
		isim = str.substr(0, adres);//noktanýn adresine kadar olan kýsmý isim deðiþkenine atacak

		filtre.open(isim+"_filtered.txt", ios::out);//dosya ismini çýktý dosya yazdýrýyoruz

		for (int j = 0; j < satir; j++)//dosyaya yazdýrma iþlemini gerçekleþtiriyoruz
		{
			for (int k = 0; k < sutun; k++)
			{
				int hucre=(j*sutun)+k;
				int a=cikismatris[hucre];
				filtre<<a;
				if(k<sutun-1)//sonda tab olmasýný istemediðimden böyle bir koþul koydum
					filtre<<"\t";
			}
			if(j<satir-1)//son satýrda boþ bir satýr olmamasý için yine bu koþulu koydum
				filtre<<"\n";
		}
		delete [] girismatris;//dinamik dizileri siliyoruz
		delete [] cikismatris;
		delete [] kismiDizialt;
		delete [] kismiDiziust;

		dosya.close();//dosyalarý kapattýk
		filtre.close();

	}
	delete [] kismiDizi;//dinamik dizi siliniyor
	delete [] kismiDizi2;
	MPI_Finalize();//mpi rutinini bitiriyoruz

	return 0;//THE END
}
