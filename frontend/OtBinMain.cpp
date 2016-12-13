#include "bloomFilterMain.h"
#include "Network/BtEndpoint.h" 

#include "OPPRF/OPPRFReceiver.h"
#include "OPPRF/OPPRFSender.h"
#include "OPPRF/BarkOPRFReceiver.h"
#include "OPPRF/BarkOPRFSender.h"

#include <fstream>
using namespace osuCrypto;
#include "util.h"

#include "Common/Defines.h"
#include "NChooseOne/KkrtNcoOtReceiver.h"
#include "NChooseOne/KkrtNcoOtSender.h"

#include "NChooseOne/Oos/OosNcoOtReceiver.h"
#include "NChooseOne/Oos/OosNcoOtSender.h"
#include "Common/Log.h"
#include "Common/Log1.h"
#include "Common/Timer.h"
#include "Crypto/PRNG.h"
#include <numeric>

//#define OOS
#define pows  { 16/*8,12,,20*/ }
#define threadss {1/*1,4,16,64*/}
#define  numTrial 5
std::vector<block> sendSet;

void BarkOPRSend()
{
	Log::out << "dsfds" << Log::endl;

    setThreadName("CP_Test_Thread");
    u64 numThreads(1);

    std::fstream online, offline;
    online.open("./online.txt", online.trunc | online.out);
    offline.open("./offline.txt", offline.trunc | offline.out);
   

    std::cout << "role  = sender (" << numThreads << ") otBin" << std::endl;

    std::string name("psi");

    BtIOService ios(0);
    BtEndpoint sendEP(ios, "localhost", 1213, true, name);

    std::vector<Channel*> sendChls_(numThreads);

    for (u64 i = 0; i < numThreads; ++i)
    {
        sendChls_[i] = &sendEP.addChannel("chl" + std::to_string(i), "chl" + std::to_string(i));
    }
    u8 dummy[1];

    senderGetLatency(*sendChls_[0]);
    sendChls_[0]->resetStats();

    LinearCode code;
   // code.loadBinFile(SOLUTION_DIR "/../libOTe/libOTe/Tools/bch511.bin");

    //for (auto pow : {/* 8,12,*/ 16/*, 20 */ })
    for (auto pow : pows)
    {

        for (auto cc : threadss)
        {
            std::vector<Channel*> sendChls;

            if (pow == 8)
                cc = std::min(8, cc);

            //std::cout << "numTHreads = " << cc << std::endl;

            sendChls.insert(sendChls.begin(), sendChls_.begin(), sendChls_.begin() + cc);

            u64 offlineTimeTot(0);
            u64 onlineTimeTot(0);
            //for (u64 numThreads = 1; numThreads < 129; numThreads *= 2)
            for (u64 jj = 0; jj < numTrial; jj++)
            {

                //u64 repeatCount = 1;
                u64 setSize = (1 << pow), psiSecParam = 40;
                PRNG prng(_mm_set_epi32(4253465, 3434565, 234435, 23987045));


                
                sendSet.resize(setSize);

                for (u64 i = 0; i < setSize; ++i)
                {
                    sendSet[i] = prng.get<block>();
                }

				std::cout << "s\n";
					std::cout << sendSet[5] << std::endl;

#ifdef OOS
                OosNcoOtReceiver otRecv(code);
                OosNcoOtSender otSend(code);
#else
                OPPRFReceiver otRecv;
                KkrtNcoOtSender otSend;
#endif
				BarkOPRFSender sendPSIs;

                //gTimer.reset();

                sendChls[0]->asyncSend(dummy, 1);
                sendChls[0]->recv(dummy, 1);
                u64 otIdx = 0;
                //std::cout << "sender init" << std::endl;
                sendPSIs.init(setSize, psiSecParam,128, sendChls,otSend, prng.get<block>());
				//std::cout << "s\n";
			//	std::cout << otSend.mGens[5].mSeed << std::endl;
				

                //return;
                sendChls[0]->asyncSend(dummy, 1);
                sendChls[0]->recv(dummy, 1);
                //std::cout << "sender init done" << std::endl;

              sendPSIs.sendInput(sendSet, sendChls);

			//  sendPSIs.mBins.print();

                u64 dataSent = 0;
                for (u64 g = 0; g < sendChls.size(); ++g)
                {
                    dataSent += sendChls[g]->getTotalDataSent();
                }

                //std::accumulate(sendChls[0]->getTotalDataSent())

                //std::cout << setSize << "    " << dataSent / std::pow(2, 20) << " byte  " << std::endl;
                for (u64 g = 0; g < sendChls.size(); ++g)
                    sendChls[g]->resetStats();

                //std::cout << gTimer << std::endl;
            }

        }


    }
    for (u64 i = 0; i < numThreads; ++i)
    {
        sendChls_[i]->close();// = &sendEP.addChannel("chl" + std::to_string(i), "chl" + std::to_string(i));
    }
    //sendChl.close();
    //recvChl.close();

    sendEP.stop();

    ios.stop();
}

void BarkOPRFRecv()
{

    setThreadName("CP_Test_Thread");
    u64 numThreads(1);

    std::fstream online, offline;
    online.open("./online.txt", online.trunc | online.out);
    offline.open("./offline.txt", offline.trunc | offline.out);
   

    std::string name("psi");

    BtIOService ios(0);
    BtEndpoint recvEP(ios, "localhost", 1213, false, name);

    LinearCode code; 

 //   code.loadBinFile(SOLUTION_DIR "/../libOTe/libOTe/Tools/bch511.bin");

    std::vector<Channel*> recvChls_(numThreads);
    for (u64 i = 0; i < numThreads; ++i)
    {
        recvChls_[i] = &recvEP.addChannel("chl" + std::to_string(i), "chl" + std::to_string(i));
    }

    std::cout << "role  = recv(" << numThreads << ") otBin" << std::endl;
    u8 dummy[1];
    recverGetLatency(*recvChls_[0]);

    //for (auto pow : {/* 8,12,*/16/*,20*/ })
    for (auto pow : pows)
    {
        for (auto cc : threadss)
        {
            std::vector<Channel*> recvChls;

            if (pow == 8)
                cc = std::min(8, cc);

            u64 setSize = (1 << pow), psiSecParam = 40;

            std::cout << "numTHreads = " << cc << "  n=" << setSize << std::endl;

            recvChls.insert(recvChls.begin(), recvChls_.begin(), recvChls_.begin() + cc);

            u64 offlineTimeTot(0);
            u64 onlineTimeTot(0);
            //for (u64 numThreads = 1; numThreads < 129; numThreads *= 2)
            for (u64 jj = 0; jj < numTrial; jj++)
            {

                //u64 repeatCount = 1;
                PRNG prng(_mm_set_epi32(42553465, 343452565, 2364435, 23923587));


                std::vector<block> recvSet(setSize);




                for (u64 i = 0; i < setSize; ++i)
                {
					  recvSet[i] = prng.get<block>();
						 // sendSet[i];// = prng.get<block>();
                }
				for (u64 i = 1; i < 3; ++i)
				{
					recvSet[i] = sendSet[i];
				}

				for (u64 i = setSize-3; i < setSize; ++i)
				{
					recvSet[i] = sendSet[i];
				}

				std::cout << "s\n";
				std::cout << recvSet[5] << std::endl;
#ifdef OOS
                OosNcoOtReceiver otRecv(code);
                OosNcoOtSender otSend(code);
#else
                KkrtNcoOtReceiver otRecv;
#endif
                BarkOPRFReceiver recvPSIs;


                recvChls[0]->recv(dummy, 1);
                gTimer.reset();
                recvChls[0]->asyncSend(dummy, 1);

                u64 otIdx = 0;


                Timer timer;
                auto start = timer.setTimePoint("start");
                recvPSIs.init(setSize, psiSecParam,128,  recvChls, otRecv, ZeroBlock);

				/*std::cout << "r\n";
				std::cout << otRecv.mGens[5][0].mSeed << std::endl;
				std::cout << otRecv.mGens[5][1].mSeed << std::endl;*/

                //return;


                //std::vector<u64> sss(recvChls.size());
                //for (u64 g = 0; g < recvChls.size(); ++g)
                //{
                //    sss[g] =  recvChls[g]->getTotalDataSent();
                //}

                recvChls[0]->asyncSend(dummy, 1);
                recvChls[0]->recv(dummy, 1);
                auto mid = timer.setTimePoint("init");


                recvPSIs.sendInput(recvSet, recvChls);
				//recvPSIs.mBins.print();


                auto end = timer.setTimePoint("done");

                auto offlineTime = std::chrono::duration_cast<std::chrono::milliseconds>(mid - start).count();
                auto onlineTime = std::chrono::duration_cast<std::chrono::milliseconds>(end - mid).count();


                offlineTimeTot += offlineTime;
                onlineTimeTot += onlineTime;
                //auto byteSent = recvChls[0]->getTotalDataSent() *recvChls.size();

                u64 dataSent = 0;
                for (u64 g = 0; g < recvChls.size(); ++g)
                {
                    dataSent += recvChls[g]->getTotalDataSent();
                    //std::cout << "chl[" << g << "] " << recvChls[g]->getTotalDataSent() << "   " << sss[g] << std::endl;
                }

                double time = offlineTime + onlineTime;
                time /= 1000;
                auto Mbps = dataSent * 8 / time / (1 << 20);

                std::cout << setSize << "  " << offlineTime << "  " << onlineTime << "        " << Mbps << " Mbps      " << (dataSent / std::pow(2.0, 20)) << " MB" << std::endl;

                for (u64 g = 0; g < recvChls.size(); ++g)
                    recvChls[g]->resetStats();

                //std::cout << "threads =  " << numThreads << std::endl << timer << std::endl << std::endl << std::endl;


                //std::cout << numThreads << std::endl;
                //std::cout << timer << std::endl;

             //   std::cout << gTimer << std::endl;

                //if (recv.mIntersection.size() != setSize)
                //    throw std::runtime_error("");







            }



            online << onlineTimeTot / numTrial << "-";
            offline << offlineTimeTot / numTrial << "-";

        }
    }

    for (u64 i = 0; i < numThreads; ++i)
    {
        recvChls_[i]->close();// = &recvEP.addChannel("chl" + std::to_string(i), "chl" + std::to_string(i));
    }
    //sendChl.close();
    //recvChl.close();

    recvEP.stop();

    ios.stop();
}

void OPPRFSend()
{
	Log::out << "dsfds" << Log::endl;

	setThreadName("CP_Test_Thread");
	u64 numThreads(1);

	std::fstream online, offline;
	online.open("./online.txt", online.trunc | online.out);
	offline.open("./offline.txt", offline.trunc | offline.out);
	


	std::cout << "role  = sender (" << numThreads << ") otBin" << std::endl;

	std::string name("psi");

	BtIOService ios(0);
	BtEndpoint sendEP(ios, "localhost", 1213, true, name);

	std::vector<Channel*> sendChls_(numThreads);

	for (u64 i = 0; i < numThreads; ++i)
	{
		sendChls_[i] = &sendEP.addChannel("chl" + std::to_string(i), "chl" + std::to_string(i));
	}
	u8 dummy[1];

	senderGetLatency(*sendChls_[0]);
	sendChls_[0]->resetStats();

	LinearCode code;
	// code.loadBinFile(SOLUTION_DIR "/../libOTe/libOTe/Tools/bch511.bin");

	//for (auto pow : {/* 8,12,*/ 16/*, 20 */ })
	for (auto pow : pows)
	{

		for (auto cc : threadss)
		{
			std::vector<Channel*> sendChls;

			if (pow == 8)
				cc = std::min(8, cc);

			//std::cout << "numTHreads = " << cc << std::endl;

			sendChls.insert(sendChls.begin(), sendChls_.begin(), sendChls_.begin() + cc);

			u64 offlineTimeTot(0);
			u64 onlineTimeTot(0);
			//for (u64 numThreads = 1; numThreads < 129; numThreads *= 2)
			for (u64 jj = 0; jj < numTrial; jj++)
			{

				//u64 repeatCount = 1;
				u64 setSize = (1 << pow), psiSecParam = 40;
				PRNG prng(_mm_set_epi32(4253465, 3434565, 234435, 23987045));



				sendSet.resize(setSize);

				for (u64 i = 0; i < setSize; ++i)
				{
					sendSet[i] = prng.get<block>();
				}

				std::cout << "s\n";
				std::cout << sendSet[5] << std::endl;

#ifdef OOS
				OosNcoOtReceiver otRecv(code);
				OosNcoOtSender otSend(code);
#else
				OPPRFReceiver otRecv;
				KkrtNcoOtSender otSend;
#endif
				OPPRFSender sendPSIs;

				//gTimer.reset();

				sendChls[0]->asyncSend(dummy, 1);
				sendChls[0]->recv(dummy, 1);
				u64 otIdx = 0;
				//std::cout << "sender init" << std::endl;
				sendPSIs.init(setSize, psiSecParam, 128, sendChls, otSend, prng.get<block>());
				//std::cout << "s\n";
				//	std::cout << otSend.mGens[5].mSeed << std::endl;


				//return;
				sendChls[0]->asyncSend(dummy, 1);
				sendChls[0]->recv(dummy, 1);
				//std::cout << "sender init done" << std::endl;

				//sendPSIs.sendInput(sendSet.data(), sendSet.size(), sendChls);
				sendPSIs.sendInput(sendSet, sendChls);

				//  sendPSIs.mBins.print();

				u64 dataSent = 0;
				for (u64 g = 0; g < sendChls.size(); ++g)
				{
					dataSent += sendChls[g]->getTotalDataSent();
				}

				//std::accumulate(sendChls[0]->getTotalDataSent())

				//std::cout << setSize << "    " << dataSent / std::pow(2, 20) << " byte  " << std::endl;
				for (u64 g = 0; g < sendChls.size(); ++g)
					sendChls[g]->resetStats();

				//std::cout << gTimer << std::endl;
			}

		}


	}
	for (u64 i = 0; i < numThreads; ++i)
	{
		sendChls_[i]->close();// = &sendEP.addChannel("chl" + std::to_string(i), "chl" + std::to_string(i));
	}
	//sendChl.close();
	//recvChl.close();

	sendEP.stop();

	ios.stop();
}

void OPPRFRecv()
{

	setThreadName("CP_Test_Thread");
	u64 numThreads(1);

	std::fstream online, offline;
	online.open("./online.txt", online.trunc | online.out);
	offline.open("./offline.txt", offline.trunc | offline.out);
	

	std::string name("psi");

	BtIOService ios(0);
	BtEndpoint recvEP(ios, "localhost", 1213, false, name);

	LinearCode code;

	//   code.loadBinFile(SOLUTION_DIR "/../libOTe/libOTe/Tools/bch511.bin");

	std::vector<Channel*> recvChls_(numThreads);
	for (u64 i = 0; i < numThreads; ++i)
	{
		recvChls_[i] = &recvEP.addChannel("chl" + std::to_string(i), "chl" + std::to_string(i));
	}

	std::cout << "role  = recv(" << numThreads << ") otBin" << std::endl;
	u8 dummy[1];
	recverGetLatency(*recvChls_[0]);

	//for (auto pow : {/* 8,12,*/16/*,20*/ })
	for (auto pow : pows)
	{
		for (auto cc : threadss)
		{
			std::vector<Channel*> recvChls;

			if (pow == 8)
				cc = std::min(8, cc);

			u64 setSize = (1 << pow), psiSecParam = 40;

			std::cout << "numTHreads = " << cc << "  n=" << setSize << std::endl;

			recvChls.insert(recvChls.begin(), recvChls_.begin(), recvChls_.begin() + cc);

			u64 offlineTimeTot(0);
			u64 onlineTimeTot(0);
			//for (u64 numThreads = 1; numThreads < 129; numThreads *= 2)
			for (u64 jj = 0; jj < numTrial; jj++)
			{

				//u64 repeatCount = 1;
				PRNG prng(_mm_set_epi32(42553465, 343452565, 2364435, 23923587));


				std::vector<block> recvSet(setSize);




				for (u64 i = 0; i < setSize; ++i)
				{
					recvSet[i] = prng.get<block>();
					// sendSet[i];// = prng.get<block>();
				}
				for (u64 i = 1; i < 3; ++i)
				{
					recvSet[i] = sendSet[i];
				}

				for (u64 i = setSize - 3; i < setSize; ++i)
				{
					recvSet[i] = sendSet[i];
				}

				std::cout << "s\n";
				std::cout << recvSet[5] << std::endl;
#ifdef OOS
				OosNcoOtReceiver otRecv(code);
				OosNcoOtSender otSend(code);
#else
				KkrtNcoOtReceiver otRecv;
#endif
				OPPRFReceiver recvPSIs;


				recvChls[0]->recv(dummy, 1);
				gTimer.reset();
				recvChls[0]->asyncSend(dummy, 1);

				u64 otIdx = 0;


				Timer timer;
				auto start = timer.setTimePoint("start");
				recvPSIs.init(setSize, psiSecParam, 128, recvChls, otRecv, ZeroBlock);

				/*std::cout << "r\n";
				std::cout << otRecv.mGens[5][0].mSeed << std::endl;
				std::cout << otRecv.mGens[5][1].mSeed << std::endl;*/

				//return;


				//std::vector<u64> sss(recvChls.size());
				//for (u64 g = 0; g < recvChls.size(); ++g)
				//{
				//    sss[g] =  recvChls[g]->getTotalDataSent();
				//}

				recvChls[0]->asyncSend(dummy, 1);
				recvChls[0]->recv(dummy, 1);
				auto mid = timer.setTimePoint("init");


			//	recvPSIs.sendInput(recvSet.data(), recvSet.size(), recvChls);
				recvPSIs.sendInput(recvSet, recvChls);
				//recvPSIs.mBins.print();


				auto end = timer.setTimePoint("done");

				auto offlineTime = std::chrono::duration_cast<std::chrono::milliseconds>(mid - start).count();
				auto onlineTime = std::chrono::duration_cast<std::chrono::milliseconds>(end - mid).count();


				offlineTimeTot += offlineTime;
				onlineTimeTot += onlineTime;
				//auto byteSent = recvChls[0]->getTotalDataSent() *recvChls.size();

				u64 dataSent = 0;
				for (u64 g = 0; g < recvChls.size(); ++g)
				{
					dataSent += recvChls[g]->getTotalDataSent();
					//std::cout << "chl[" << g << "] " << recvChls[g]->getTotalDataSent() << "   " << sss[g] << std::endl;
				}

				double time = offlineTime + onlineTime;
				time /= 1000;
				auto Mbps = dataSent * 8 / time / (1 << 20);

				std::cout << setSize << "  " << offlineTime << "  " << onlineTime << "        " << Mbps << " Mbps      " << (dataSent / std::pow(2.0, 20)) << " MB" << std::endl;

				for (u64 g = 0; g < recvChls.size(); ++g)
					recvChls[g]->resetStats();

				//std::cout << "threads =  " << numThreads << std::endl << timer << std::endl << std::endl << std::endl;


				//std::cout << numThreads << std::endl;
				//std::cout << timer << std::endl;

				//   std::cout << gTimer << std::endl;

				//if (recv.mIntersection.size() != setSize)
				//    throw std::runtime_error("");







			}



			online << onlineTimeTot / numTrial << "-";
			offline << offlineTimeTot / numTrial << "-";

		}
	}

	for (u64 i = 0; i < numThreads; ++i)
	{
		recvChls_[i]->close();// = &recvEP.addChannel("chl" + std::to_string(i), "chl" + std::to_string(i));
	}
	//sendChl.close();
	//recvChl.close();

	recvEP.stop();

	ios.stop();
}

//void OPPRF_EmptrySet_Test_Impl1()
//{
//	u64 setSize = 2 << 8, psiSecParam = 40, bitSize = 128;
//	PRNG prng(_mm_set_epi32(4253465, 3434565, 234435, 23987045));
//
//	std::vector<block> sendSet(setSize), recvSet(setSize);
//	for (u64 i = 0; i < setSize; ++i)
//	{
//		sendSet[i] = prng.get<block>();
//		recvSet[i] = prng.get<block>();
//		//recvSet[i] = sendSet[i];
//	}
//	for (u64 i = 1; i < 3; ++i)
//	{
//		recvSet[i] = sendSet[i];
//	}
//
//	std::string name("psi");
//
//	BtIOService ios(0);
//	BtEndpoint ep0(ios, "localhost", 1212, true, name);
//	BtEndpoint ep1(ios, "localhost", 1212, false, name);
//
//
//	std::vector<Channel*> recvChl{ &ep1.addChannel(name, name) };
//	std::vector<Channel*> sendChl{ &ep0.addChannel(name, name) };
//
//	KkrtNcoOtReceiver otRecv;
//	KkrtNcoOtSender otSend;
//
//
//	//u64 baseCount = 128 * 4;
//	//std::vector<std::array<block, 2>> sendBlks(baseCount);
//	//std::vector<block> recvBlks(baseCount);
//	//BitVector choices(baseCount);
//	//choices.randomize(prng);
//
//	//for (u64 i = 0; i < baseCount; ++i)
//	//{
//	//    sendBlks[i][0] = prng.get<block>();
//	//    sendBlks[i][1] = prng.get<block>();
//	//    recvBlks[i] = sendBlks[i][choices[i]];
//	//}
//
//	//otRecv.setBaseOts(sendBlks);
//	//otSend.setBaseOts(recvBlks, choices);
//
//	//for (u64 i = 0; i < baseCount; ++i)
//	//{
//	//    sendBlks[i][0] = prng.get<block>();
//	//    sendBlks[i][1] = prng.get<block>();
//	//    recvBlks[i] = sendBlks[i][choices[i]];
//	//}
//
//
//	OPPRFSender send;
//	OPPRFReceiver recv;
//	std::thread thrd([&]() {
//
//
//		send.init(setSize, psiSecParam, bitSize, sendChl, otSend, prng.get<block>());
//		send.sendInput(sendSet, sendChl);
//		//Log::out << sendSet[0] << Log::endl;
//		//	send.mBins.print();
//
//
//	});
//
//	recv.init(setSize, psiSecParam, bitSize, recvChl, otRecv, ZeroBlock);
//	recv.sendInput(recvSet, recvChl);
//	//Log::out << recvSet[0] << Log::endl;
//	//recv.mBins.print();
//
//
//	thrd.join();
//
//	sendChl[0]->close();
//	recvChl[0]->close();
//
//	ep0.stop();
//	ep1.stop();
//	ios.stop();
//}

