#include<stdio.h>
#include<iostream>
#include<cstring>
#include<unistd.h>
#include<cstdlib>
#include<thread>
#include<fstream>
#include<vector>
#include<sstream>
#include<utility>
#include<conio.h>
#include<ctype.h>
#include<math.h>

/* 
 * Asumsi aplikasi
 * > Jalur layanan 1 : khusus untuk kendaraan besar
 * > Jalur layanan 2-4 : bisa untuk kendaraan besar dan kecil
 * > Menggunakan 1 queue untuk 4 jalur layanan, secara bergiliran mengisi setiap jalur layanan yang tersedia.
 * 
 * Sistem dipakai aplikasi
 * > Menggunakan thread sebagai proses dari waktu pengerjaan
 * > Ada 2 konfigurasi automasi untuk semua jalur layanan : auto_assign_lane & auto_finish
 * > Struct yang dibuat : data_mobil, tnode(linked list queue), slot(jalur layanan/bay)
 * 
 * Penyimpanan hasil transaksi yang selesai akan disimpan di file db.txt
 * dengan format CSV(comma-seperated) dengan keterangan kolom sebagai berikut: {no_plat},{jenis_mobil},{nama_kendaraan},{layanan},{nilai_transaksi}
 * contoh: D000AB,1,Hino,1|0|0,70000
 * 
 * Dicompile melalui Dev C++ menggunakan gcc dengan compiler option "-static-libgcc  -std=c++11"
 */

using namespace std;

//waktu yang dibutuhkan untuk menyelesaikan satu mobil kecil/sedang
int time_to_finish[3] = {20,10,10}; //cuci, vakum, poles

//lokasi data untuk menyimpan transaksi
string database_location = "db.txt";

//worker status (thread loop condition)
bool worker_status = true;

//konfigurasi apakah ingin aplikasi langsung memroses antrian mobil langsung ke jalur layanan yg tersedia
bool auto_assign_lane = true;

//konfigurasi apakah ingin aplikasi otomatis menyelesaikan transaksi yang ada (mengosongkan jalur yang sudah selesai)
bool auto_finish = true;


//tipe data mobil
struct data_mobil
{
	char plat[7];
	int jenis_mobil;
	char nama_kendaraan[9];
	int layanan[3];
};

//tipe data node single linked list
struct tnode{
	struct data_mobil *value;
	struct tnode* next;
};
struct tnode *head = NULL, *temp = NULL;

//tipe data untuk jalur layanan
struct slot{
	int id;
	bool active =  true;
	bool kendaraan_besar = false;
	struct data_mobil *mobil = NULL;
	int time_elapsed = 0;
};

//mengembalikan node yang berada di index ke x di linked list
tnode *get_at_index(int id){
	if( id < 0 ){
		return NULL;
	}
	int i = 0;
	temp = head;
	while( i < id ){
		if( temp->next != NULL ){
			temp = temp->next;
		}else{
			printf("Index out of range.\n");
			return NULL;
		}
		i++;
	}
	return temp;
}

//menambahkan node pada ujung linked list
void insert_last(data_mobil *x){
	struct tnode *node = (struct tnode*) malloc(sizeof(struct tnode));
	node->value = x; 
	node->next = NULL;
	if (head != NULL){
		temp = head;
		while( temp->next != NULL ){
			temp = temp->next;
		}
		temp->next = node;
	}else{
		head = node;
	}
}

//menghapus node yang berada di index ke x di linked list
void delete_at_index(int id){
	if (head != NULL){
		if( id != 0 ){
			temp = get_at_index(id-1);
			temp->next = temp->next->next;
		}else{
			head = head->next;
		}
	}
}

//menambahkan antrian mobil
void queue(char plat[7], int jenis_mobil, char nama_kendaraan[9], int layanan[3]){
	struct data_mobil *mobil = (struct data_mobil*) malloc(sizeof(struct data_mobil));
	strcpy(mobil->plat,plat);
	strcpy(mobil->nama_kendaraan,nama_kendaraan);
	mobil->jenis_mobil = jenis_mobil;
	for(int i = 0; i < 3 ; i++){
		mobil->layanan[i] = layanan[i];
	}
	insert_last(mobil);
}

//mengambil antrian mobil
data_mobil *dequeue(){
	if( head != NULL ){
		struct data_mobil *mobil = (struct data_mobil*) malloc(sizeof(struct data_mobil));
		mobil = head->value;
		delete_at_index(0);
		return mobil;
	}
	return NULL;
}

//menghitung waktu total layanan
int hitung_waktu(data_mobil *x){
	int hasil = 0;
	if(x->layanan[0] > 0){
		hasil += time_to_finish[0];
	}
	if(x->layanan[1] > 0){
		hasil += time_to_finish[1];
	}
	if(x->layanan[2] > 0){
		hasil += time_to_finish[2];
	}
	if( x->jenis_mobil == 3 || x->jenis_mobil == 4 ){
		return hasil*2;
	}else{
		return hasil;
	}
}

//menghitung biaya layanan
int hitung_biaya(data_mobil *x){
	int hasil = 0;
	for(int i = 0; i < 3 ; i++){
		if( x->layanan[i] > 0 ){
  			switch(x->jenis_mobil){
  				case 1: 
  					if( i == 0 ){
  						hasil += 50000;
					}else if( i == 1 ){
  						hasil += 35000;
					}else{
  						hasil += 125000;
					}
				break;
				case 2: 
  					if( i == 0 ){
  						hasil += 60000;
					}else if( i == 1 ){
  						hasil += 40000;
					}else{
  						hasil += 150000;
					}
				break;
				case 3: 
  					if( i == 0 ){
  						hasil += 70000;
					}else if( i == 1 ){
  						hasil += 50000;
					}else{
  						hasil += 200000;
					}
				break;
				case 4: 
  					if( i == 0 ){
  						hasil += 70000;
					}else if( i == 1 ){
  						hasil += 40000;
					}
				break;
			}
		}
	}
	return hasil;
}

//format harga menjadi ada separator
string number_format(string value, char thousandSep = '.'){
    int len = value.length();
    int dlen = 3;

    while (len > dlen) {
        value.insert(len - dlen, 1, thousandSep);
        dlen += 4;
        len += 1;
    }
    return value;
}

//untuk print string yang sudah dialign : left/center
string add_whitespace(string source, int length_target, string align ="left"){
	if( align == "left" ){
		for(int i = source.length() ; i <= length_target ; i++){
			source += " ";
		}
	}else if( align == "center" ){
		int l = source.length();
		int pos = floor(((length_target-l)/2));
		int pos2 = length_target-(pos + l );
		for(int i = 0; i < pos ; i++){
			source = " "+source;
		}
		for(int i = 0; i < pos2 ; i++){
			source += " ";
		}
	}else if( align == "right" ){
		for(int i = source.length() ; i <= length_target ; i++){
			source = " "+source;
		}
	}
	return source;
}

/*print semua antrian
* parameter: max untuk limit jumlah queue yg tampil, title untuk mengubah judul tabel
*/
void get_queue(int max = 0, string title = "Queue"){
	temp = head;
	int no = 0;
	printf("|===================================================================================|\n");
	cout << "|" << add_whitespace(title,83,"center") << "|" << endl;
	printf("|===================================================================================|\n");
	printf("|      |          |    |Jenis  |Nama      | Layanan               | Total  | Total  |\n");
	printf("| No   |   Plat   |Kode|mobil  |Kendaraan |-----------------------| Waktu  | Harga  |\n");
	printf("|      |          |    |       |          | Cuci  | Vakum | Poles |        | (Rp)   |\n");
	printf("|------|----------|----|-------|----------|-------|-------|-------|--------|--------|\n");
	while( temp !=  NULL ){
		no++;
		string jenis = "";
		switch(temp->value->jenis_mobil){
			case 1: jenis = "Kecil"; break;
			case 2: jenis = "Sedang"; break;
			case 3: jenis = "Besar"; break;
			case 4: jenis = "Truck"; break;
		}
		char cuci = temp->value->layanan[0] > 0 ? 'v' : 'x'; 
		char vakum = temp->value->layanan[1] > 0 ? 'v' : 'x'; 
		char poles = temp->value->layanan[2] > 0 ? 'v' : 'x'; 
		cout << "|" << add_whitespace(to_string(no),5) << "|";
		cout << add_whitespace(string(temp->value->plat),9) << "|";
		printf("%i   |",temp->value->jenis_mobil);
		cout << add_whitespace(jenis,6) << "|";
		cout << add_whitespace(temp->value->nama_kendaraan,9) << "|";
		printf("   %c   |",cuci);
		printf("   %c   |",vakum);
		printf("   %c   |",poles);
		cout << add_whitespace(to_string(hitung_waktu(temp->value)),7) << "|";
		cout << add_whitespace(number_format(to_string(hitung_biaya(temp->value))),7) << "|" << endl;
		if( temp->next == NULL ){
			break;
		}
		temp = temp->next;
		if( no == max && no > 0 ){
			break;
		}
	}
	if( no == 0 ){
		cout << "Tidak ada antrian" << endl;
	}
	printf("|===================================================================================|\n");
}

//mencetak daftar harga
void print_tabel_harga(){
	printf("|=================================================|\n");
	printf("|Kode|Jenis kendaraan     |       Harga           |\n");
	printf("|    |                    |-----------------------|\n");
	printf("|    |                    |  Cuci | Vakum | Poles |\n");
	printf("|----|--------------------|-----------------------|\n");
	printf("|1   |Kecil: ayla,jazz    |50.000 |35.000 |125.000|\n");
	printf("|2   |Sedan: MPV, avanza, |60.000 |40.000 |150.000|\n");
	printf("|    |rush,brv            |       |       |       |\n");
	printf("|3   |Besar: fortuner,    |70.000 |50.000 |200.000|\n");
	printf("|    |alphard             |       |       |       |\n");
	printf("|4   |Truck               |70.000 |40.000 |   -   |\n");
	printf("|=================================================|\n");
}

// memecah string menjadi vector bedasarkan delimiter
vector<string> explode(string const & s, char delim){
    vector<string> result;
    istringstream iss(s);

    for (string token; getline(iss, token, delim); )
    {
        result.push_back(move(token));
    }

    return result;
}

//mengeluarkan semua hasil transaksi dari file txt
void getReport(){
	printf("|==========================================================================|\n");
	printf("|                        Hasil transaksi                                   |\n");
	printf("|==========================================================================|\n");
	printf("|      |          |    |Jenis  |Nama      | Layanan               | Total  |\n");
	printf("| No   |   Plat   |Kode|mobil  |Kendaraan |-----------------------| Harga  |\n");
	printf("|      |          |    |       |          | Cuci  | Vakum | Poles | (Rp)   |\n");
	printf("|------|----------|----|-------|----------|-------|-------|-------|--------|\n");
	int total_transaksi = 0;
	ifstream ifile(database_location);
	if (ifile) {
		string tmp;
		int no = 1;
	  	while (getline (ifile, tmp)) {
	  		if( tmp == "" ){
	  			continue;	
			}
		  	vector<string> data = explode(tmp, ',');
		  	vector<string> layanan = explode(data[3], '|');
		  	string jenis = "";
			switch(stoi(data[1])){
				case 1: jenis = "Kecil"; break;
				case 2: jenis = "Sedang"; break;
				case 3: jenis = "Besar"; break;
				case 4: jenis = "Truck"; break;
			}
			char cuci = stoi(layanan[0]) > 0 ? 'v' : 'x'; 
			char vakum = stoi(layanan[1]) > 0 ? 'v' : 'x'; 
			char poles = stoi(layanan[2]) > 0 ? 'v' : 'x'; 
			cout << "|" << add_whitespace(to_string(no),5) << "|";
			cout << add_whitespace(data[0],9) << "|";
			printf("%i   |",stoi(data[1]));
			cout << add_whitespace(jenis,6) << "|";
			cout << add_whitespace(data[2],9) << "|";
			printf("   %c   |",cuci);
			printf("   %c   |",vakum);
			printf("   %c   |",poles);
			cout << add_whitespace(number_format(data[4]),7) << "|" << endl;
			total_transaksi += stoi(data[4]);
			no++;
	  	}
	}
	ifile.close();
	printf("|==========================================================================|\n");
	printf("|===============================| Total Transaksi | Rp ");
	cout << add_whitespace(number_format(to_string(total_transaksi)),19,"right") << "|" << endl;
	printf("|==========================================================================|\n");
}

// mencatat mobil yang selesai ke file
void logReport(data_mobil *x){
	string layanan = "";
	int hasil = hitung_biaya(x);
	for(int i = 0; i < 3 ; i++){
		layanan += to_string(x->layanan[i]);
		if( i < 2 ){
			layanan += "|";
		}
	}
	string fformat = string(x->plat)+","+to_string(x->jenis_mobil)+","+x->nama_kendaraan+","+layanan+","+to_string(hasil);
	ofstream ofile;
	ofile.open(database_location, ios_base::app);
	ofile << fformat << endl;
	ofile.close();
}

//print status dari sebuah jalur layanan
void report_status(struct slot *x){
	printf("=======================\n");
	printf("Bay No %i\n",x->id);
	if( x->mobil == NULL ){
		printf("Status: Kosong\n");
	}else{
		printf("Plat: %s\n",x->mobil->plat);
		cout << "Nama kendaraan: " << x->mobil->nama_kendaraan << endl;
		int waktu_selesai = hitung_waktu(x->mobil);
		if( x->mobil->jenis_mobil == 3  || x->mobil->jenis_mobil == 4 ){
			printf("Tipe: Kendaraan Besar/Truck\n");
		}else{
			printf("Tipe: Kendaraan Kecil/Sedang\n");
		}
		if( x->time_elapsed >= waktu_selesai ){
			printf("Status: Selesai\n");
		}else{
			printf("Status: Dalam pengerjaan\n");
		}
		printf("Time Elapsed: %i\n",x->time_elapsed);
		printf("Waktu yang dibutuhkan: %i\n",waktu_selesai);
	}
	printf("=======================\n");
}

//mengecek apakah suatu jalur layanan sudah selesai
bool check_is_finish(struct slot *x){
	if( x->mobil != NULL ){
		if( x->time_elapsed >= hitung_waktu(x->mobil) ){
			return true;
		}
		return false;
	}
	return true;
}

//mengambil mobil dari antrian ke jalur layanan
data_mobil *get_new_vehicle(struct slot *x){
	struct tnode *node = (struct tnode*) malloc(sizeof(struct tnode));
	node = get_at_index(0);
	if( node != NULL ){
		if( x->kendaraan_besar ){
			if( node->value->jenis_mobil == 3 || node->value->jenis_mobil == 4 ){
				//ok
				return dequeue();
			}
		}else{
			//ok
			return dequeue();
		}
	}
	return NULL;
}

//Menyelesaikan suatu pembayaran / jalur
bool finish_lane(struct slot *x){
	bool finish = check_is_finish(x);
	if( finish && x->mobil != NULL ){
		logReport(x->mobil);
		x->time_elapsed = 0;
		x->mobil = NULL;
		return true;
	}
	return false;
}

//Assign queue secara manual
void next_queue(struct slot *x[]){
	bool ok_lane[4] = {false,false,false,false};
	struct tnode *node = get_at_index(0); 
	if( node == NULL ){
		cout << "No queue available." << endl;
		return;
	}
	get_queue(1,"Next Queue");
	cout << endl << endl;
	cout << "================================" << endl;
	cout << "Available lane:" << endl;
	for(int i = 0; i < 4; i++){
		cout << "Bay #" << x[i]->id;
		if( check_is_finish(x[i]) ){
			if( x[i]->kendaraan_besar ){
				if( node->value->jenis_mobil == 3 || node->value->jenis_mobil == 4 ){
					cout << ": Tersedia";
					ok_lane[i] = true;
					if( x[i]->mobil != NULL ){
						cout << " (Ada mobil)";
					}
				}else{
					cout << ": Jenis tidak didukung";
				}
			}else{
				cout << ": Tersedia";
				ok_lane[i] = true;
				if( x[i]->mobil != NULL ){
					cout << " (Ada mobil)";
				}
			}
		}else{
			cout << ": Penuh";
		}
		cout << endl;
	}
	cout << "================================" << endl;
	int select_lane = 0;
	cout << "Catatan: jika anda memilih jalur yang sudah SELESAI namun terdapat mobil, maka jika anda memilih akan langsung menyelesaikan transaksi sebelumnya." << endl;
	cout << "Assign next queue to lane (enter non-existent lane to cancel): ";
	cin >> select_lane;
	if( select_lane < 1 || select_lane > 4 ){
		cout << "Operation cancelled." << endl;
		return;
	}
	if( !check_is_finish(x[select_lane-1]) || !ok_lane[select_lane-1] ){
		cout << "Cannot assign to the selected lane (occupied/incompatible)." << endl;
		return;
	}
	finish_lane(x[select_lane-1]);
	x[select_lane-1]->mobil = get_new_vehicle(x[select_lane-1]);
	cout << "Queue assign success." << endl;
	return;
}

//Menu untuk memilih transaksi yang ingin dibereskan (manual mode)
void select_finish(struct slot *x[]){
	bool ok_lane[4] = {false,false,false,false};
	cout << "Transaksi yang dapat selesai:" << endl;
	for(int i = 0 ; i < 4 ; i++){
		if( check_is_finish(x[i]) && x[i]->mobil != NULL ){
			printf("========================\n");
			printf("Slot No %i\n",x[i]->id);
			printf("Plat: %s\n",x[i]->mobil->plat);
			cout << "Nama kendaraan: " << x[i]->mobil->nama_kendaraan << endl;
			if( x[i]->mobil->jenis_mobil == 3  || x[i]->mobil->jenis_mobil == 4 ){
				printf("Tipe: Kendaraan Besar/Truck\n");
			}else{
				printf("Tipe: Kendaraan Kecil/Sedang\n");
			}
			cout << "Harga: Rp " << number_format(to_string(hitung_biaya(x[i]->mobil))) << endl;
			printf("Time Elapsed: %i\n",x[i]->time_elapsed);
			printf("========================\n");
			ok_lane[i] = true;
		}
	}
	int select_lane;
	cout << "Select lane transaction to finish (enter non-existent lane to cancel): ";
	cin >> select_lane;
	if( select_lane < 1 || select_lane > 4 ){
		cout << "Operation cancelled." << endl;
		return;
	}
	if( !ok_lane[select_lane-1] ){
		cout << "Selected lane isn't ready to finish transaction." << endl;
		return;
	}
	finish_lane(x[select_lane-1]);
	cout << "Transaction success." << endl;
	return;
}

//Loop theread untuk setiap jalur
void slot_loop(struct slot *x){
	if( x->active ){
		bool finish = check_is_finish(x);
		if( finish ){
			if( auto_finish ){
				finish_lane(x);
			}
		}else if( x->mobil != NULL ){
			x->time_elapsed++;
		}
		if( x->mobil == NULL && auto_assign_lane ){
			//take new vehicle
			x->mobil = get_new_vehicle(x);
		}
	}
}

//Loop untuk jalur layanan
void cuci_worker(struct slot *x[]){
	while(worker_status){
		for(int i = 0; i < 4; i++){
			slot_loop(x[i]);
		}
		this_thread::sleep_for(chrono::seconds(1));
	}
}

//print semua keadaan detail slot
void print_all_slot(struct slot *x[]){
	for(int i = 0; i < 4; i++){
		report_status(x[i]);
	}
}

//Penambah press any to continue ke menu CLI
void press_enter_to_continue(){
	cout << "Press any to continue.." << endl;
	_getch();
}

//Menu about dan welcome screen
void welcomeScreen(){
	printf("|=================================|\n");
	printf("|     Cuci Mobil \"WASH-WASH\"      |\n");
	printf("| Compatible only with Windows OS |\n");
	printf("| Made by Matthew Christopher A.  |\n");
	printf("| NIM: 2301848981                 |\n");
	printf("|=================================|\n");
	press_enter_to_continue();
}

// Main Program
int main(){
	//konfigurasi setting aplikasi
	worker_status = true;
	auto_assign_lane = false;
	auto_finish = false;
	
	welcomeScreen();
	
	//inisialisasi slot
	struct slot slot1;
	slot1.id = 1;
	slot1.kendaraan_besar = true;
	struct slot slot2;
	slot2.id = 2;
	struct slot slot3;
	slot3.id = 3;
	struct slot slot4;
	slot4.id = 4;
	struct slot *mySlot[] = {&slot1,&slot2,&slot3,&slot4};
	
	//menjalankan thread/jalur pelayanan
	thread worker(cuci_worker,mySlot);
	
	//looping menu
	int choice = -1;
	while( choice != 0 ){
		system("cls");
		printf("|==========================|\n");
		printf("| Cuci Mobil \"WASH-WASH\"   |\n");
		printf("|==========================|\n");
		printf("|1. Daftar Antrian         |\n");
		printf("|2. Tambahkan Antrian      |\n");
		printf("|3. Antrian Selanjutnya    |\n");
		printf("|4. Selesaikan transaksi   |\n");
		printf("|5. Detail semua jalur     |\n");
		printf("|6. Semua transaksi        |\n");
		printf("|7. Daftar harga           |\n");
		printf("|8. Settings               |\n");
		printf("|9. About                  |\n");
		printf("|0. Exit                   |\n");
		printf("|==========================|\n");
		choice = -1;
		while( choice < 0 || choice > 9 ){
			printf("Your choice: ");
			scanf("%i",&choice);
		}
		system("cls");
		if( choice == 1 ){
			// Melihat daftar antrian yg ada
			get_queue();
			press_enter_to_continue();
		}else if( choice == 2 ){
			// Menambah antrian mobil
			cout << "Create new queue:" << endl;
			string plat, nama;
			int kode;
			char tmp = 'n'; 
			int layanan[3] = {0,0,0};
			cout << "Plat: ";
			cin >> plat;
			cout << endl;
			if( plat.length() < 3 || plat.length() > 7 ){
				cout << "Warning: Invalid Plat" << endl;
				press_enter_to_continue();
				continue;
			}
			cout << "1: Kecil, 2: Sedang, 3: Besar, 4: Truck" << endl;
			cout << "Kode jenis mobil: ";
			cin >> kode;
			cout << endl;
			if( kode < 1 || kode > 4 ){
				cout << "Warning: Invalid Kode Mobil" << endl;
				press_enter_to_continue();
				continue;
			}
			cout << "Nama kendaraan (maksimal 9 karakter): ";
			cin >> nama;
			cout << endl;
			if( nama.length() < 1 || nama.length() > 9 ){
				cout << "Warning: Invalid Nama Kendaraan" << endl;
				press_enter_to_continue();
				continue;
			}
			cout << "Cuci (y/n)?: ";
			cin >> tmp;
			layanan[0] = tolower(tmp) == 'y'? 1 : 0;
			cout << endl;
			cout << "Vakum (y/n)?: ";
			cin >> tmp;
			layanan[1] = tolower(tmp) == 'y'? 1 : 0;
			cout << endl;
			if( kode != 4 ){
				cout << "Poles (y/n)?: ";
				cin >> tmp;
				layanan[2] = tolower(tmp) == 'y'? 1 : 0;
				cout << endl;
			}
			int lay_count = 0;
			for(int i = 0 ; i< 3 ; i++){
				lay_count += layanan[i] > 0 ? 1 : 0;
			}
			if( lay_count == 0 ){
				cout << "Warning: Tolong pilih salah satu layanan yang tersedia." << endl;
				press_enter_to_continue();
				continue;
			}
			char plat_c[7], nama_c[9];
			strcpy(plat_c, plat.c_str());
			strcpy(nama_c, nama.c_str());
			queue(plat_c,kode,nama_c,layanan);
			cout << "Antrian berhasil ditambahkan." << endl;
			press_enter_to_continue();
			
		}else if( choice == 3 ){
			//Mengambil mobil dari queue terdepan untuk dimasukan ke jalur (manual assign mode)
			next_queue(mySlot);
			press_enter_to_continue();
		}else if( choice == 4 ){
			//Memilih mobil yang ingin diselesaikan transaksinya (manual finish mode)
			select_finish(mySlot);
			press_enter_to_continue();
		}else if( choice == 5 ){
			//Melihat keadaan detail status dari setiap jalur layanan / bay
			print_all_slot(mySlot);
			press_enter_to_continue();
		}else if( choice == 6 ){
			// Melihat semua hasil transaksi yang tercatat di file txt
			getReport();
			press_enter_to_continue();
		}else if( choice == 7 ){
			// Melihat daftar harga layanan dalam bentuk tabel
			print_tabel_harga();
			press_enter_to_continue();
		}else if( choice == 8 ){
			// Mengganti setting aplikasi
			cout << "==================================================" << endl;
			cout << "                Program Settings" << endl;
			cout << "                0 = off, 1 = on" << endl;
			cout << "==================================================" << endl;
			cout << "No. |" << endl;
			cout << "====|=============================================" << endl;
			cout << " 1  | Auto Assign Lane (mengambil mobil dari antrian secara otomatis ke jalur): " << auto_assign_lane << endl;
			cout << " 2  | Auto Finish (otomatis selesaikan transaksi): " << auto_finish << endl;
			cout << "==================================================" << endl;
			cout << "Toggle session setting no (select nonexistent no to abort): ";
			int choice;
			cin >> choice;
			if( choice < 1 || choice > 2 ){
				cout << "Operation cancelled" << endl;
				continue;
			}
			switch(choice){
				case 1:
					auto_assign_lane = auto_assign_lane ? false : true;
				break;
				case 2:
					auto_finish = auto_finish ? false : true;
				break;
			}
			cout << "Setting changed." << endl;
			press_enter_to_continue();
		}else if( choice == 9 ){
			welcomeScreen();
		}
	}
	cout << "Thank you for using." << endl;
	worker_status = false;
	worker.join();
	return 0;
}
