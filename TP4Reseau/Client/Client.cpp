// Client.cpp : Defines the entry point for the console application.
//
#include "stdafx.h"
#undef UNICODE 
#define _WINSOCK_DEPRECATED_NO_WARNINGS

#include <winsock2.h>
#include <ws2tcpip.h>
#include <stdlib.h>
#include <stdio.h>
#include <iostream>
#include <string> 


// Link avec ws2_32.lib
#pragma comment(lib, "ws2_32.lib")
using namespace std;

int __cdecl main(int argc, char **argv)
{
	WSADATA wsaData;
	SOCKET leSocket;// = INVALID_SOCKET;
	struct addrinfo *result = NULL,
		*ptr = NULL,
		hints;
	char motEnvoye[200];
	char* motEnvoyePtr = &motEnvoye[400];
	char motRecu[200];
	int iResult;

	//--------------------------------------------
	// InitialisATION de Winsock
	iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
	if (iResult != 0) {
		printf("Erreur de WSAStartup: %d\n", iResult);
		return 1;
	}
	// On va creer le socket pour communiquer avec le serveur
	leSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (leSocket == INVALID_SOCKET) {
		printf("Erreur de socket(): %ld\n\n", WSAGetLastError());
		freeaddrinfo(result);
		WSACleanup();
		printf("Appuyez une touche pour finir\n");
		getchar();
		return 1;
	}
	//--------------------------------------------
	// On va chercher l'adresse du serveur en utilisant la fonction getaddrinfo.
	ZeroMemory(&hints, sizeof(hints));
	hints.ai_family = AF_INET;        // Famille d'adresses
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_protocol = IPPROTO_TCP;  // Protocole utilisé par le serveur

									  // On indique le nom et le port du serveur auquel on veut se connecter
									  //char *host = "L4708-XX";
									  //char *host = "L4708-XX.lerb.polymtl.ca";
									  //char *host = "add_IP locale";
	// Adresse IP Carl
	//char host[50] = "192.168.0.105";
	cout << "CLIENT" << endl;
	cout << "-----------------------------------------------------" << endl;
	//Recuperation de l'adresse locale	
	bool goodIP = false;
	string sPort;
	const char * port;
	int iPort;
	string host = "";
	while (!goodIP)
	{
		printf("Saisir l'adresse IP de la machine serveur : ");
		cin >> host;
		struct sockaddr_in sa;
		int result = InetPton(AF_INET, host.c_str(), &(sa.sin_addr));
		if (result == 1)
			goodIP = true;
		else
			printf("Mauvais format d'adresse IP");
	}
	
	do
	{
		cout << "Le port doit etre un chiffre entre 5000 et 5050" << endl;
		cout << "Saisir le port de la machine serveur : ";
		if (!(cin >> sPort)) {
			cin.clear();
			cin.ignore(999, '\n');
		}
		iPort = stoi(sPort);
		//cout << port << " and " << iPort << endl;
	}while (!(iPort >= 5000 && iPort <= 5050));
	port = sPort.c_str();
	cin.clear();
	

	//cout << endl << *cPort << " ainsi que " << to_string(port) << endl;

	// getaddrinfo obtient l'information du host en question
	iResult = getaddrinfo(host.c_str(), port, &hints, &result);
	if (iResult != 0) {
		printf("Erreur de getaddrinfo: %d\n", iResult);
		WSACleanup();
		return 1;
	}
	//---------------------------------------------------------------------		
	//On parcours les adresses retournees jusqu'a trouver la premiere adresse IPV4
	while ((result != NULL) && (result->ai_family != AF_INET))
		result = result->ai_next;

	//	if ((result != NULL) &&(result->ai_family==AF_INET)) result = result->ai_next;  

	//-----------------------------------------
	if (((result == NULL) || (result->ai_family != AF_INET))) {
		freeaddrinfo(result);
		printf("Impossible de recuperer la bonne adresse\n\n");
		WSACleanup();
		printf("Appuyez une touche pour finir\n");
		getchar();
		return 1;
	}

	sockaddr_in *adresse;
	adresse = (struct sockaddr_in *) result->ai_addr;
	//----------------------------------------------------
	cout << "Adresse trouvee pour le serveur " + host + " : " + inet_ntoa(adresse->sin_addr) << endl;

	//Demande au client du nom d'utilisateur
	string nomUtilisateur;
	cout << "Nom d'utilisateur : ";
	cin >> nomUtilisateur;
	
	//Demande du mot de passe 
	string motDePasse;
	cout << "Mot de passe : ";
	cin >> motDePasse;

	cout << "Tentative de connexion au serveur ";
	cout << inet_ntoa(adresse->sin_addr);
	cout << " avec le port ";
	cout << port << endl;

	// On va se connecter au serveur en utilisant l'adresse qui se trouve dans
	// la variable result.
	iResult = connect(leSocket, result->ai_addr, (int)(result->ai_addrlen));
	if (iResult == SOCKET_ERROR) {
		printf("Impossible de se connecter au serveur %s sur le port %s\n\n", inet_ntoa(adresse->sin_addr), port);
		freeaddrinfo(result);
		WSACleanup();
		printf("Appuyez une touche pour finir\n");
		getchar();
		return 1;
	}

	cout << "Connecte au serveur " << host << ":" << port << endl;
	freeaddrinfo(result);
	
	//Verification de l'utilisateur et du mot de passe
	string verificationStr = "connection\n" + nomUtilisateur + "\n" + motDePasse;
	const char *verification = verificationStr.c_str();

	iResult = send(leSocket, verification, strlen(verification), 0);
	if (iResult == SOCKET_ERROR) {
		printf("Erreur du send: %d\n", WSAGetLastError());
		closesocket(leSocket);
		WSACleanup();
		printf("Appuyez une touche pour finir\n");
		getchar();

		return 1;
	}

	//----------------------------
	// Demander à l'usager un mot a envoyer au serveur, continuellement
	cin.clear();
	cin.ignore(999, '\n');
	const char *message;
	string messageStr; 

	while (true) {
		bool messageCorrect = false;
		while (!messageCorrect) {
			cout << "Saisir le message a envoyer (max 200 caracteres) : ";
			cin.getline(motEnvoye, 400);
			if (strlen(motEnvoye) > 200) {
				cout << "Erreur : message trop long. " << endl;
				char *begin = motEnvoye;
				char *end = begin + sizeof(motEnvoye);
				fill(begin, end, 0);
			}
			else messageCorrect = true;
		}
		//-----------------------------
		// Envoyer le mot au serveur
		messageStr = "message\n" + nomUtilisateur + "\n";
		messageStr.append(motEnvoye);
		message = messageStr.c_str();
		iResult = send(leSocket, message, strlen(message), 0);
		if (iResult == SOCKET_ERROR) {
			printf("Erreur du send: %d\n", WSAGetLastError());
			closesocket(leSocket);
			WSACleanup();
			printf("Appuyez une touche pour finir\n");
			getchar();

			return 1;
		}
		//clear du mot envoye
		char *begin = motEnvoye;
		char *end = begin + sizeof(motEnvoye);
		fill(begin, end, 0);
		messageStr.clear();

		printf("Nombre d'octets envoyes : %ld\n", iResult);

		//------------------------------
		// Maintenant, on va recevoir l' information envoyée par le serveur
		iResult = recv(leSocket, motRecu, 200, 0);
		if (iResult > 0) {
			printf("Nombre d'octets recus: %d\n", iResult);
			motRecu[iResult] = '\0';
			printf("Le mot recu est %*s\n", iResult, motRecu);
		}
		else {
			printf("Erreur de reception : %d\n", WSAGetLastError());
		}
		begin = motRecu;
		end = begin + sizeof(motRecu);
		fill(begin, end, 0);
	}
	// cleanup
	closesocket(leSocket);
	WSACleanup();

	printf("Appuyez une touche pour finir\n");
	getchar();
	return 0;
}