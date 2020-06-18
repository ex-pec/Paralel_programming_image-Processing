#include <mpi.h>
#include <iostream>
#include <cstdlib>
#include <sstream>
#include <string>
#include <fstream>

using namespace std;

int ortadeger(int dizi[])//dizimizi sÄ±ralamak iÃ§in ayrÄ± bir fonksiyon kullanÄ±yoruz
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
	return dizi[4];//dizi geriye ortanca terimini dÃ¶ndÃ¼rÃ¼r
}
int main(int argc,char *argv[])
{	
	int size,myRank,boyut,kismiSatir,localHucre,localDeger,kismiSutun;//makinalar arası iletişim için gerekli olan değişkenler

	double startTime,endTime,elapsedTime;//zaman hesaplaması için gerekli olan değişkenler

	unsigned char *kismiDizi,*girismatris,*cikismatris;//dinamik dizi pointerlarÄ± 
	
	int s;//dosyadan okuduÄŸumuz deÄŸerle geÃ§ici olarak bu deÄŸiÅŸkene atÄ±yoruz
	int intdizi[9];//diziyi sÄ±ralamak iÃ§in 9 boyutlu bir dizi tanÄ±mladÄ±k
	int satir = 0, sutun = 0,hucre;//dizi dinamik boyut deÄŸiÅŸkenleri tanÄ±mladÄ±k 
	
	MPI_Status status;//haberleşmemiz için gerekli olan mpi ritüeli 

	MPI_Init(&argc,&argv);//mpi başlatıyoruz
	
	MPI_Comm_size(MPI_COMM_WORLD,&size);//toplam ağdaki makina sayısını alıyoruz
	MPI_Comm_rank(MPI_COMM_WORLD,&myRank);//kendi numarasını bu değişkende tutacak her pc
	
	
	if(myRank==0)//eğer ana makina ise yapılacak işlemler
	{	
		string str = argv[1];//dosya adÄ±nÄ± almak iÃ§in deÄŸiÅŸkene argÃ¼manÄ±mÄ±zÄ± atÄ±yoruz
		string nokta = ".";//uzantÄ±yÄ± almamak iÃ§in spliti noktadan alÄ±yoruz
		int adres = 0;//noktanÄ±n adresini tutmak iÃ§in deÄŸiÅŸkenimiz
		string isim;//uzantÄ±sÄ±z isim iÃ§in deÄŸiÅŸkenimiz
		adres = str.find(nokta);
		isim = str.substr(0, adres);//noktanÄ±n adresine kadar olan kÄ±smÄ± isim deÄŸiÅŸkenine atacak

		ifstream dosya;//dosyayÄ± okuma modunda aÃ§Ä±yoruz
		ofstream filtre;//dosyayÄ± yazma modunda aÃ§Ä±yoruz

		dosya.open(argv[1], ios::in);//dosyayÄ± aÃ§Ä±yoruz
		filtre.open(isim+"_filtered.txt", ios::out);//dosya ismini Ã§Ä±ktÄ± dosya yazdÄ±rÄ±yoruz

		cout << "Dosya okunuyor..."<<endl;
		dosya >> satir;	//dizi boyutunu alÄ±yoruz
		dosya >> sutun;
		
		if(((satir-2)%size)!=0)
		{
			cout<<"Goruntu matrisi esit dagitilamiyor.\n";
			MPI_Abort(MPI_COMM_WORLD,99);
		}
		
		girismatris=new unsigned char[satir*sutun];//dinamik matrislerimizi ayarladık
		cikismatris=new unsigned char[satir*sutun];
		
		for (int j = 0; j < satir; j++)//ardÄ±ndan okuma iÅŸlemi 2. satÄ±rdan devam ediyor ve tÃ¼m terimleri matrislerimizin iÃ§ine dolduruyoruz.
		{
			for (int k = 0; k < sutun; k++)
			{
				dosya >> s;
				hucre=j*sutun+k;
				girismatris[hucre]=s;
				cikismatris[hucre] = girismatris[hucre];
			}
		}

		startTime=MPI_Wtime();//zamanlayÄ±cÄ± mpi.h kÃ¼tÃ¼phanesi yardÄ±mÄ±yla baÅŸlatÄ±yoruz
		
		kismiSatir=(satir-2)/size;//satır bazında bölümlendirme gerçekleştirmek istediğimiz için alıyoruz değişkenimize
		for(int i=1;i<size;i++)//diğer makinalara dizi için gerekli verileri gönderdik 
		{
			MPI_Send(&kismiSatir,1,MPI_INT,i,25,MPI_COMM_WORLD);
			MPI_Send(&sutun,1,MPI_INT,i,25,MPI_COMM_WORLD);
		}
		for(int i=1;i<size;i++)//dizileri gönderdik diğer makinalara
			MPI_Send(&girismatris[i*kismiSatir*sutun],((kismiSatir+2)*sutun),MPI_UNSIGNED_CHAR,i,25,MPI_COMM_WORLD);

		//ilk makina iÃ§in iÅŸlemleri yaptÄ±rÄ±yoruz
		int z=0;
		for (int f = 0; f < kismiSatir; f++, z = z - 3)//satÄ±rlarÄ± gezer
		{
			int q = f + 3, p = z + 3;
			for (int z = 0; z < sutun - 2; z++, f = f - 3)//sÃ¼tunlarÄ± gezer 
			{
				int q = f + 3, p = z + 3, n = 0;
				while (f<q)//3x3 matris burda geÃ§ici dizimize atÄ±lÄ±yor
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

		for(int i=1;i<size;i++)//ana makina olarak diğer makinalarda hesaplanmış matris verisini ve hücresini alıyoruz
		{	
			for(int j=0;j<kismiSatir*(sutun-2);j++)
			{	
				MPI_Recv(&localHucre,1,MPI_INT,i,25,MPI_COMM_WORLD,&status);
				MPI_Recv(&localDeger,1,MPI_INT,i,25,MPI_COMM_WORLD,&status);
				cikismatris[localHucre]=localDeger;
			}
		}

		endTime=MPI_Wtime();//zamanlayıcıyı bitiriyoruz
		elapsedTime=endTime-startTime;//geçen zamanı buluyoruz
		cout<<endl<<"Toplam hesaplama icin gecen sure= "<<elapsedTime<<endl;

		for (int j = 0; j < satir; j++)//dosyaya yazdÄ±rma iÅŸlemini gerÃ§ekleÅŸtiriyoruz
		{
			for (int k = 0; k < sutun; k++)
			{
				int hucre=(j*sutun)+k;
				int a=cikismatris[hucre];
				filtre<<a;
				if(k<sutun-1)//sonda tab olmasÄ±nÄ± istemediÄŸimden bÃ¶yle bir koÅŸul koydum
					filtre<<"\t";
			}
			if(j<satir-1)//son satÄ±rda boÅŸ bir satÄ±r olmamasÄ± iÃ§in yine bu koÅŸulu koydum
				filtre<<"\n";
		}
		delete [] girismatris;//dinamik dizileri siliyoruz
		delete [] cikismatris;
		dosya.close();
		filtre.close();
	}
	else//ana makina değilse yapılacak işlemler 
	{
		MPI_Recv(&kismiSatir,1,MPI_INT,0,25,MPI_COMM_WORLD,&status);//satir bilgisini alıyoruz
		MPI_Recv(&sutun,1,MPI_INT,0,25,MPI_COMM_WORLD,&status);//sutun bilgisini alıyoruz
		kismiSatir+=2;//hesaplanması gereken satırdan 2 satır fazlası gerekli o yüzden dizi boyutunu ona göre aldık
		kismiDizi=new unsigned char[kismiSatir*sutun];//dinamik kısmi dizimiz oluşturuldu

		MPI_Recv(kismiDizi,kismiSatir*sutun,MPI_UNSIGNED_CHAR,0,25,MPI_COMM_WORLD,&status);//diziyi alıyoruz	
		
		int z=0;//hesaplama işlemleri
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
				//hücre değerini makinadan bu şekilde bir denklemle doğru bir şekilde alabiliyoruz
				MPI_Send(&localHucre,1,MPI_INT,0,25,MPI_COMM_WORLD);//verileri ana makinaya gönderiyoruz
				MPI_Send(&localDeger,1,MPI_INT,0,25,MPI_COMM_WORLD);
			}
		}	
		delete [] kismiDizi;//dinamik dizi siliniyor
	}

	MPI_Finalize();//mpi rutinini bitiriyoruz
	return 0;//THE END
}
