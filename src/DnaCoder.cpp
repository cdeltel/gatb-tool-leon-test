
#include "DnaCoder.hpp"

/*
#define PRINT_DEBUG_EXTMUTA
#define PRINT_DEBUG_ENCODER
#define PRINT_DEBUG_DECODER
*/


char bin2NTrev[4] = {'T','G','A','C'};
//char bin2NT[4] = {'A','C','T','G'};

/*
GACGCGCCGATATAACGCGCTTTCCCGGCTTTTACCACGTCGTTGAGGGCTTCCAGCGTCTCTTCGATCGGCGTGTTGTAATCCCAGCGATGAATTTG6:2308q
			Anchor pos: 8
			Anchor: GATATAACGCGCTTTCCCGGCTTTTACCACG
			§ Anchor: 8
*/

//====================================================================================
// ** AbstractDnaCoder
//====================================================================================
AbstractDnaCoder::AbstractDnaCoder(Leon* leon) :
_kmerModel(leon->_kmerSize, KMER_DIRECT),
_anchorPosSizeModel(8),
_readTypeModel(2), //only 2 value in this model: read with anchor or without anchor
_noAnchorReadModel(5), _mutationModel(5), //5value: A, C, G, T, N
_noAnchorReadSizeModel(8),
_readSizeModel(8),
_readAnchorRevcompModel(2)
{
	_leon = leon;
	_bloom = _leon->_bloom;
	_kmerSize = _leon->_kmerSize;
	
	
	for(int i=0; i<8; i++){
		_anchorAdressModel.push_back(Order0Model(256));
		_anchorPosModel.push_back(Order0Model(256));
		_noAnchorReadSizeValueModel.push_back(Order0Model(256));
		_readSizeValueModel.push_back(Order0Model(256));
	}
	
	
}

void AbstractDnaCoder::startBlock(){
	for(int i=0; i<8; i++){
		_anchorAdressModel[i].clear();
		_anchorPosModel[i].clear();
		_noAnchorReadSizeValueModel[i].clear();
		_readSizeValueModel[i].clear();
	}
	_anchorPosSizeModel.clear();
	_readTypeModel.clear();
	_noAnchorReadModel.clear();
	_mutationModel.clear();
	_noAnchorReadSizeModel.clear();
	_readSizeModel.clear();
	_readAnchorRevcompModel.clear();
		
}
/*
void AbstractDnaCoder::codeSeedBin(KmerModel* model, kmer_type* kmer, int nt, bool right){
	return codeSeedNT(model, kmer, Leon::bin2nt(nt), right);
}

void AbstractDnaCoder::codeSeedNT(KmerModel* model, kmer_type* kmer, char nt, bool right){
	string kmerStr = kmer->toString(_kmerSize);
	//cout << kmerStr << " " << nt << " " << right << endl;
	if(right){
		kmerStr += nt;
		kmerStr.erase(kmerStr.begin());
		*kmer = model->codeSeed(kmerStr.c_str(), Data::ASCII);
		
	}
	else{		
		kmerStr.insert(kmerStr.begin(), nt);
		kmerStr.pop_back();
		*kmer = model->codeSeed(kmerStr.c_str(), Data::ASCII);
	}
		
		//cout << "\t" << kmerStr << endl;
		//cout << "\t" << kmer->toString(_kmerSize) << endl;
}
*/

void AbstractDnaCoder::codeSeedBin(KmerModel* model, kmer_type* kmer, int nt, bool right){
	//if(nt == 4) nt = 0;
	//string kmerStr = "AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAN"
	//kmer_type kmer2 = model->codeSeed(kmerStr.c_str(), Data::ASCII);
	//cout << kmer2->toString(_kmerSize) << endl;
	
	if(right){
		*kmer = model->codeSeedRight(*kmer, nt, Data::INTEGER);
	}
	else{
		kmer_type kmer_end_rev = revcomp(*kmer, _kmerSize);
		*kmer = model->codeSeedRight (kmer_end_rev, binrev[nt], Data::INTEGER);
		*kmer = revcomp(*kmer, _kmerSize);
	}
}

void AbstractDnaCoder::codeSeedNT(KmerModel* model, kmer_type* kmer, char nt, bool right){
	//if(nt == 'N') nt = 'A';
	return codeSeedBin(model, kmer, Leon::nt2bin(nt), right);
}

//====================================================================================
// ** DnaEncoder
//====================================================================================
DnaEncoder::DnaEncoder(Leon* leon) :
AbstractDnaCoder(leon), _itKmer(_kmerModel)
{
}

DnaEncoder::DnaEncoder(const DnaEncoder& copy) :
AbstractDnaCoder(copy._leon), _itKmer(_kmerModel)
{
	//_leon = copy._leon;
	//_bloom = copy._bloom;
}

DnaEncoder::~DnaEncoder(){
	writeBlock();
}

void DnaEncoder::operator()(Sequence& sequence){
	_sequence = &sequence;
	//cout << _sequence->getIndex() << endl;
	_readSize = _sequence->getDataSize();
	_readseq = _sequence->getDataBuffer();
	
	_leon->_totalDnaSize += _readSize; //unsynch
	
	
	//_lastSequenceIndex = sequence->getIndex();
	
	if(_sequence->getIndex() % Leon::READ_PER_BLOCK == 0){
		writeBlock();
		startBlock();
	}
	
	execute();
}

void DnaEncoder::writeBlock(){
	if(_rangeEncoder.getBufferSize() > 0){
		_rangeEncoder.flush();
	}
	
	_leon->_realDnaCompressedSize += _rangeEncoder.getBufferSize();
	_leon->writeBlock(_rangeEncoder.getBuffer(), _rangeEncoder.getBufferSize());
	_rangeEncoder.clear();
	
	/*
	cout << "------------------------" << endl;
	cout << "Read count:    " << _leon->_readCount << endl;
	cout << "Anchor kmer count:   " << _leon->_anchorKmerCount << endl;
	cout << "Read per anchor:    " << _leon->_readCount / _leon->_anchorKmerCount << endl;
	cout << "Bit per anchor: " << log2(_leon->_anchorKmerCount);
	cout << endl;
	cout << "Read without anchor: " << (_leon->_readWithoutAnchorCount*100) / _leon->_readCount << "%" << endl;
	cout << "\tWith N: " << (_leon->_noAnchor_with_N_kmer_count*100) / _leon->_readWithoutAnchorCount << "%" << endl;
	cout << "\tFull N: " << (_leon->_noAnchor_full_N_kmer_count*100) / _leon->_readWithoutAnchorCount << "%" << endl;
	cout << endl;*/
	//cout << "Total kmer: " <<  _leon->_total_kmer << endl;
	//cout << "Kmer in bloom:  " << (_leon->_total_kmer_indexed*100) / _leon->_total_kmer << "%" << endl;
	//cout << "Uniq mutated kmer: " << (_leon->_uniq_mutated_kmer*100) / _leon->_total_kmer << "%" << endl;
	//cout << "anchor Hash:   memory_usage: " << _anchorKmers.memory_usage() << "    memory_needed: " << _anchorKmers.size_entry ()*_leon->_anchorKmerCount << " (" << (_anchorKmers.size_entry ()*_leon->_anchorKmerCount*100) / _anchorKmers.memory_usage() << "%)" << endl;
	
}

void DnaEncoder::execute(){
	
	//if(_leon->_readCount > 18) return;
	//cout << endl << "\tEncoding seq " << _sequence->getIndex() << endl;
	//cout << "\t\t" << _readseq << endl;
	#ifdef PRINT_DEBUG_ENCODER
		cout << endl << "\tEncoding seq " << _sequence->getIndex() << endl;
		cout << "\t\t" << _readseq << endl;
	#endif
	
	//cout << _readseq << endl;
	
	_leon->_readCount += 1;
	
	if(_readSize < _kmerSize){
		encodeNoAnchorRead();
		return;
	}
	 
	//cout << _leon->_readCount << endl;
	//kmer_type anchorKmer = 0;
	u_int32_t anchorAdress;
	
	buildKmers();
	int anchorPos = findExistingAnchor(&anchorAdress);
	
	if(anchorPos == -1)
		anchorPos = _leon->findAndInsertAnchor(_kmers, &anchorAdress);
	
	//cout << anchorPos << endl;
	
	if(anchorPos == -1)
		encodeNoAnchorRead();
	else{
		encodeAnchorRead(anchorPos, anchorAdress);
	}
	
}

void DnaEncoder::buildKmers(){

	
	_Npos.clear();
	
	for(int i=0; i<_readSize; i++){
		if(_readseq[i] == 'N'){
			_Npos.push_back(i);
			_readseq[i] = 'A';
		}
	}
		
	/*
	bool lala = false;
	for(int i=0; i<_readSize; i++){
		if(_readseq[i] == 'N'){
			lala = true;
			break;
		}
	}
	if(lala){
		cout << "lala" << endl;
		cout << string(_readseq).substr(0, _readSize) << endl;
		
		for(int i=0; i<_readSize; i++){
			if(_readseq[i] == 'N'){
				_Npos.push_back(i);
				_readseq[i] = 'A';
				lala = true;
			}
		}
		
		_itKmer.setData(_sequence->getData());
		for (_itKmer.first(); !_itKmer.isDone(); _itKmer.next()){
			cout << (*_itKmer).toString(_kmerSize) << endl;
		}
		

	
	}
	*/
	_itKmer.setData(_sequence->getData());
	
	_kmers.clear();
	for (_itKmer.first(); !_itKmer.isDone(); _itKmer.next()){
		//cout << (*_itKmer).toString(_kmerSize) << endl;
		_kmers.push_back(*_itKmer);
	}
	
	//if(_sequence->getIndex() == 53445) cout << _Npos.size() << endl;
	/*
	if(_sequence->getIndex() == 29){
		cout << string(_readseq).substr(0, _readSize) << endl;
		for (_itKmer.first(); !_itKmer.isDone(); _itKmer.next()){
			cout << (*_itKmer).toString(_kmerSize) << endl;
		}
	}*/
	
}

int DnaEncoder::findExistingAnchor(u_int32_t* anchorAdress){
	kmer_type kmer, kmerMin;
	
	for(int i=0; i<_kmers.size(); i++){
		kmer = _kmers[i];
		kmerMin = min(kmer, revcomp(kmer, _kmerSize));
		if(_leon->anchorExist(kmerMin, anchorAdress)){
			return i;
		}
	}
	return -1;
}


void DnaEncoder::encodeAnchorRead(int anchorPos, u_int32_t anchorAdress){
	#ifdef PRINT_DEBUG_ENCODER
		cout << "\t\tEncode anchor read" << endl;
	#endif
	
	_rangeEncoder.encode(_readTypeModel, 0);
	CompressionUtils::encodeNumeric(_rangeEncoder, _readSizeModel, _readSizeValueModel, _readSize);
	CompressionUtils::encodeNumeric(_rangeEncoder, _anchorPosSizeModel, _anchorPosModel, anchorPos);
	CompressionUtils::encodeFixedNumeric(_rangeEncoder, _anchorAdressModel, anchorAdress, 4);
	
	kmer_type anchor = _kmers[anchorPos];
	
	if(anchor == min(anchor, revcomp(anchor, _kmerSize)))
		_rangeEncoder.encode(_readAnchorRevcompModel, 0);
	else
		_rangeEncoder.encode(_readAnchorRevcompModel, 1);
			
	#ifdef PRINT_DEBUG_ENCODER
		cout << "\t\t\tAnchor pos: " << anchorPos << endl;
		cout << "\t\t\tAnchor: " << _kmers[anchorPos].toString(_kmerSize) << endl;
	#endif
	
	_mutations.clear();
	_uniqNoOrigMutationPos.clear();
	
	//kmer_type kmer = anchor;
	for(int i=anchorPos-1; i>=0; i--){
		i = encodeMutations(i, false);
		//kmer = encodeMutations(kmer, &i, false);
		//cout << kmer.toString(_kmerSize) << endl;
	}
	
	//kmer = anchor;
	for(int i=anchorPos+_kmerSize; i<_readSize; i++){
		//cout << "Pos: " << i << endl;
		i = encodeMutations(i, true);
	//for(int i=anchorPos; i<_kmers.size()-1; i++)
		//kmer = encodeMutations(kmer, &i, true);
		//cout << kmer.toString(_kmerSize) << endl;
	}
		
		

	CompressionUtils::encodeNumeric(_rangeEncoder, _anchorPosSizeModel, _anchorPosModel, _Npos.size());
	for(int i=0; i<_Npos.size(); i++){
		CompressionUtils::encodeNumeric(_rangeEncoder, _anchorPosSizeModel, _anchorPosModel, _Npos[i]);
	}
	
	CompressionUtils::encodeNumeric(_rangeEncoder, _anchorPosSizeModel, _anchorPosModel, _uniqNoOrigMutationPos.size());
	for(int i=0; i<_uniqNoOrigMutationPos.size(); i++){
		CompressionUtils::encodeNumeric(_rangeEncoder, _anchorPosSizeModel, _anchorPosModel, _uniqNoOrigMutationPos[i]);
	}
	
	for(int i=0; i<_mutations.size(); i++){
		//cout << Leon::nt2bin(_mutations[i]) << " ";
		_rangeEncoder.encode(_mutationModel, Leon::nt2bin(_mutations[i]));
	}
	//cout << endl;
}
	
int DnaEncoder::encodeMutations(int pos, bool rightExtend){
		
		
	if(std::find(_Npos.begin(), _Npos.end(), pos) != _Npos.end()){
		return pos;
	}
	
	kmer_type kmerMin;
	kmer_type kmer;

	if(rightExtend)
		kmer = _kmers[pos-_kmerSize];
	else
		kmer = _kmers[pos+1];
	
	//cout << "Kmer: " << kmer.toString(_kmerSize) << endl;
	
	kmer_type uniqKmer, mutatedSolidKmer;
	int uniqNt;
	char nextNt;
	bool isKmerSolid = false;
	
	nextNt = _readseq[pos];
	
	
	//cout << "Real next nt: " << nextNt << endl;
	
	//cout << nextNt << endl;
	
	//kmer = _kmers[pos];
	
	//if(kmer.toString(_kmerSize).find("N") !=  string::npos) cout << "lala" << endl;

	int indexedKmerCount = 0;
	
	for(int nt=0; nt<4; nt++){
		
		kmer_type mutatedKmer = kmer;
		codeSeedBin(&_kmerModel, &mutatedKmer, nt, rightExtend);
		kmer_type mutatedKmerMin = min(mutatedKmer, revcomp(mutatedKmer, _kmerSize));
		
		//mutatedKmer.printASCII(_kmerSize);
		
		if(_bloom->contains(mutatedKmerMin)){
			indexedKmerCount += 1;
			uniqNt = nt;
			uniqKmer = mutatedKmer;
			
			
			if(Leon::bin2nt(nt) == nextNt){
				isKmerSolid = true;
			}
		}
		
	}
	
	_leon->_MCtotal += 1;
		
	if(indexedKmerCount == 1){
		if(isKmerSolid){
			_leon->_MCuniqSolid += 1;
		}
		else{
			_leon->_MCuniqNoSolid += 1;
			_leon->_readWithAnchorMutationChoicesSize += 0.25;
			_mutations.push_back(nextNt);
			_uniqNoOrigMutationPos.push_back(pos);
		}
	}
	else{
		if(indexedKmerCount == 0){
			_leon->_MCnoAternative += 1;
		}
		else{
			if(isKmerSolid){
				_leon->_MCmultipleSolid += 1;
				
				if(_solidMutaChainLockTime <= 0){
					
					if(extendMutaChain(kmer, pos, rightExtend)){
						#ifdef PRINT_DEBUG_EXTMUTA
							cout << "\t\t" << _solidMutaChainStartPos << " " << _solidMutaChainSize << endl;
						#endif
						if(rightExtend)
							return pos + _solidMutaChainSize;
						else
							return pos - _solidMutaChainSize;
					}
					else{
						_solidMutaChainLockTime = 1;
						//cout << _solidMutaChainLockTime << endl;
						//return pos;
						/*
						//cout << _solidMutaChainSize << endl;
						if(rightExtend){
							for(int i=_solidMutaChainStartPos; i<_solidMutaChainStartPos+_solidMutaChainSize-1; i++){
								#ifdef PRINT_DEBUG_EXTMUTA
									cout << _readseq[i] << endl;
								#endif
								_mutations.push_back(_readseq[i]);
							}
							return _solidMutaChainStartPos+_solidMutaChainSize-2;
						}
						else{
							for(int i=_solidMutaChainStartPos; i>_solidMutaChainStartPos-_solidMutaChainSize+1; i--){
								#ifdef PRINT_DEBUG_EXTMUTA
									cout << _readseq[i] << endl;
								#endif
								_mutations.push_back(_readseq[i]);
							}
							return _solidMutaChainStartPos-_solidMutaChainSize+2;
						}*/
					}
				}
				else{
					if(_solidMutaChainLockTime > 0) _solidMutaChainLockTime -= 1;
				}
					
					
			}
			else{
				_leon->_MCmultipleNoSolid += 1;
			}
		}
		
		_leon->_readWithAnchorMutationChoicesSize += 0.25;
		_mutations.push_back(nextNt);
	}
	
	return pos;
	
}

bool DnaEncoder::extendMutaChain(kmer_type kmer, int pos, bool rightExtend){
	return false;
	
	#ifdef PRINT_DEBUG_EXTMUTA
		//if(_lala == 2) return false;
	#endif
	
	_solidMutaChainStartPos = pos;
			
	#ifdef PRINT_DEBUG_EXTMUTA
		cout << "Extend muta chain !!!" << endl;
		cout << "\t" << _readseq << endl;
		cout << "\tStart kmer: " << kmer.toString(_kmerSize) << endl;
	#endif
	
	vector< vector< vector<kmer_type> > > mutaChains;
	vector< vector<kmer_type> > mutas;
	for(int i=0; i<4; i++) mutas.push_back(vector<kmer_type>());
	
	for(int nt=0; nt<4; nt++){
		
		kmer_type mutatedKmer = kmer;
		codeSeedBin(&_kmerModel, &mutatedKmer, nt, rightExtend);
		kmer_type mutatedKmerMin = min(mutatedKmer, revcomp(mutatedKmer, _kmerSize));
		
		
		if(_bloom->contains(mutatedKmerMin)){
			
			#ifdef PRINT_DEBUG_EXTMUTA
				cout << "\t\tAlternatives: " << mutatedKmer.toString(_kmerSize) << endl;
			#endif
			
			mutas[nt].push_back(mutatedKmer);
			
			//extendMutaChainRec(mutatedKmer, pos, rightExtend);
		}
	}
	
	mutaChains.push_back(mutas);
	
	return extendMutaChainRec(mutaChains, rightExtend);
}

bool DnaEncoder::extendMutaChainRec(vector< vector< vector<kmer_type> > >& mutaChains, bool rightExtend){
	int pos = mutaChains.size();
	_solidMutaChainSize = mutaChains.size();
	
	if(_solidMutaChainStartPos+pos >= _readSize || _solidMutaChainStartPos-pos < 0){
		#ifdef PRINT_DEBUG_EXTMUTA
			cout << "\t\t\t\t\tRead overflow" << endl;
		#endif
		_solidMutaChainSize += 1 ;
		return false;
	}
		
		
	vector< vector<kmer_type> > mutas;
	for(int i=0; i<4; i++) mutas.push_back(vector<kmer_type>());
	
	vector< vector<kmer_type> > mutatedKmers = mutaChains[pos-1];
	
	for(int NT=0; NT<4; NT++){
		
		for(int i=0; i < mutatedKmers[NT].size(); i++){
			kmer_type kmer = mutatedKmers[NT][i];
			
			for(int nt=0; nt<4; nt++){
				
				kmer_type mutatedKmer = kmer;
				codeSeedBin(&_kmerModel, &mutatedKmer, nt, rightExtend);
				kmer_type mutatedKmerMin = min(mutatedKmer, revcomp(mutatedKmer, _kmerSize));
				
				if(_bloom->contains(mutatedKmerMin)){
					mutas[nt].push_back(mutatedKmer);
					
					#ifdef PRINT_DEBUG_EXTMUTA
						cout << "\t\t\tSuper alternatives: " << mutatedKmer.toString(_kmerSize) << endl;
					#endif
				}
			}
			
			
		}
	}
	mutaChains.push_back(mutas);
	
	_solidMutaChainSize = mutaChains.size();

	char originalNt;
	int originalNtBin;
	
	if(rightExtend)
		originalNt = _readseq[_solidMutaChainStartPos + pos];
	else
		originalNt = _readseq[_solidMutaChainStartPos - pos];
	
	originalNtBin = Leon::nt2bin(originalNt);
	
	#ifdef PRINT_DEBUG_EXTMUTA
		cout << "\t\t\t\tOriginal nt at: " << originalNt << endl;
	#endif
	
	
	if(mutaChains[pos][originalNtBin].size() == 0){
		#ifdef PRINT_DEBUG_EXTMUTA
			cout << "\t\t\t\t\tOriginal nt is not solid" << endl;
		#endif
		return false;
	}
		
	
	bool fullUniq = true;
	bool uniq = false;
	//mutas = mutaChains[mutaChains.size()-1];
	
	for(int nt=0; nt<4; nt++){
		
		if(mutas[nt].size() == 1){
			if(nt == originalNtBin){
				uniq = true;
			}
			else{
				fullUniq = false;
			}
		}
		
	}
		
	//cout << _sequence->getIndex() <<  "  Pos: " << _solidMutaChainStartPos + pos << endl;

	
	//cout << uniq << " " <<  fullUniq << endl;
	if(uniq && fullUniq){
		_solidMutaChainSize = pos;
		#ifdef PRINT_DEBUG_EXTMUTA
			cout << "\t\t\t\t\tMuta chain success" << endl;
			_lala += 1;
		#endif
		return true;
	}
	else{
		if(pos > 10){
			#ifdef PRINT_DEBUG_EXTMUTA
				cout << "\t\t\t\t\tToo much extend" << endl;
			#endif
			return false;
		}
		return extendMutaChainRec(mutaChains, rightExtend);
	}
	
	
	
}

int DnaEncoder::voteMutations(int pos, bool rightExtend){
	kmer_type kmer;
	int maxScore = 0;
	int votes[4];
	
	int bestNt;
	
	//kmer_type mutatedKmers[4];
	//vector<int> mutations;
	//bool isMutateChoice[4];
	kmer = _kmers[pos];
	/*
	for(int nt=0; nt<4; nt++){
		
		kmer_type mutatedKmer = kmer;
		codeSeedBin(&_kmerModel, &mutatedKmer, nt, rightExtend);
		kmer_type mutatedKmerMin = min(mutatedKmer, revcomp(mutatedKmer, _kmerSize));
		
		//mutated_kmer.printASCII(_kmerSize);
		
		if(_bloom->contains(mutatedKmerMin)){
			mutations.push_back(nt);
			mutatedKmers[nt] = mutatedKmer;
			votes[nt] = 0;
			//isMutateChoice[nt] = true;
		}
	}
	*/
	//kmer_type currentKmer;
	//cout << _readseq << endl;
	//cout << pos << ": " << kmer.toString(_kmerSize) << endl;
	
	for(int nt=0; nt<4; nt++){
		
		kmer_type mutatedKmer = kmer;
		codeSeedBin(&_kmerModel, &mutatedKmer, nt, rightExtend);
		kmer_type mutatedKmerMin = min(mutatedKmer, revcomp(mutatedKmer, _kmerSize));
		
		for(int j=0; j<4; j++){
			char nextNt;
			//int kmerPos;
			if(rightExtend){
				if(pos+1+_kmerSize+j >= _readSize) break;
				nextNt = _readseq[pos+1+_kmerSize+j];
			}
			else{
				if(pos-2-j < 0) break;
				nextNt = _readseq[pos-2-j];
			}
			//cout << j << ": " << nextNt << endl;
			codeSeedNT(&_kmerModel, &mutatedKmer, nextNt, rightExtend);
			kmer_type mutatedKmerMin = min(mutatedKmer, revcomp(mutatedKmer, _kmerSize));
			
			if(_bloom->contains(mutatedKmerMin)){
				votes[nt] += 1;
				if(votes[nt] > maxScore){
					maxScore = votes[nt];
					bestNt = nt;
				}
			}
		}
	}
	
	/*
	cout << "---------------------" << endl;
	for(int i=0; i<mutations.size(); i++){
		int nt = mutations[i];
		cout << bin2NT[nt] << " " << votes[nt] << endl;
	}*/
	
	if(maxScore == 0){
		//cout << "No best NT" << endl;
		bestNt = -1;
	}
	//else
		//cout << "Best nt: " << bin2NT[bestNt] << endl;
	
	return bestNt;
	
	
}

void DnaEncoder::encodeNoAnchorRead(){
	#ifdef PRINT_DEBUG_ENCODER
		cout << "\t\tEncode no anchor read" << endl;
	#endif

	//Reinsert N because they can be encoded by the coder
	for(int i=0; i<_Npos.size(); i++){
		_readseq[_Npos[i]] = 'N';
	}
	
	_rangeEncoder.encode(_readTypeModel, 1);
	
	_leon->_readWithoutAnchorSize += _readSize*0.375;
	_leon->_readWithoutAnchorCount += 1;
	
	/*
	for(int i=0; i<_readSize; i++){
		if(_readseq[i] == 'N'){
			_leon->_noAnchor_with_N_kmer_count += 1;
			break;
		}
	}
	
	bool full_N = true;
	for(int i=0; i<_readSize; i++){
		if(_readseq[i] != 'N'){
			full_N = false;
			break;
		}
	}
	if(full_N){
		_leon->_noAnchor_full_N_kmer_count += 1;
	}*/
	
	CompressionUtils::encodeNumeric(_rangeEncoder, _noAnchorReadSizeModel, _noAnchorReadSizeValueModel, _readSize);
	for(int i=0; i<_readSize; i++){
		_rangeEncoder.encode(_noAnchorReadModel, Leon::nt2bin(_readseq[i]));
	}
}







//====================================================================================
// ** DnaDecoder
//====================================================================================
DnaDecoder::DnaDecoder(Leon* leon, ifstream* inputFile, ofstream* outputFile) :
AbstractDnaCoder(leon)
{
	_inputFile = inputFile;
	_outputFile = outputFile;
	
}

DnaDecoder::~DnaDecoder(){
	//delete _rangeDecoder;
	//delete _outputFile;
}

void DnaDecoder::setup(u_int64_t blockStartPos, u_int64_t blockSize){
	startBlock();
	_rangeDecoder.clear();
	
	_inputFile->seekg(blockStartPos, _inputFile->beg);
	_rangeDecoder.setInputFile(_inputFile);
	
	_blockStartPos = blockStartPos;
	_blockSize = blockSize;
	
	#ifdef PRINT_DEBUG_DECODER
		cout << "\t-----------------------" << endl;
		cout << "\tDecoding block " << _blockStartPos << " - " << _blockStartPos+_blockSize << endl;
	#else
		cout << "|" << flush;
	#endif
	
	execute();
}

void DnaDecoder::execute(){
	
	//decodeFirstHeader();
		
	//int i=0;
	//while(i < Leon::READ_PER_BLOCK){
	while(_inputFile->tellg() < _blockStartPos+_blockSize){
		//if(_leon->_readCount > 1) return;
	
		
		u_int8_t readType = _rangeDecoder.nextByte(_readTypeModel);
		//cout << "Read type: " << (int)readType << endl;

		if(readType == 0)
			decodeAnchorRead();
		else if(readType == 1)
			decodeNoAnchorRead();
			
		endSeq();
		//cout << _inputFile->tellg() << " " << _blockStartPos+_blockSize << endl;
		/*
		string trueSeq = string((*_leon->_testBankIt)->getDataBuffer());
		trueSeq = trueSeq.substr(0, _readSize);
		//cout << trueSeq << endl;
		//cout << _currentSeq << endl;
		if(trueSeq != _currentSeq){
			cout << (*_leon->_testBankIt)->getIndex() << "\t\tseq different !!" << endl;
			cout << "\t\t" << trueSeq << endl;
			cout << "\t\t" << _currentSeq << endl;
			_leon->_readCount += 1;
			return;
		}
		_leon->_testBankIt->next();
		*/
		#ifdef PRINT_DEBUG_DECODER
			_leon->_readCount += 1;
			cout << _leon->_readCount << ": " << _currentSeq << endl;
		#endif
		
		//i++;
		//_leon->_readCount += 1;
		//if(i == 1) return;
		//_currentSeq.clear();
		
		//cout << (int)(_inputFile->tellg() < _blockStartPos+_blockSize) << endl;
	}
	
	
}

void DnaDecoder::decodeAnchorRead(){
	#ifdef PRINT_DEBUG_DECODER
		cout << "\t\tDecode anchor read" << endl;
	#endif
		
	_readSize = CompressionUtils::decodeNumeric(_rangeDecoder, _readSizeModel, _readSizeValueModel);
	int anchorPos = CompressionUtils::decodeNumeric(_rangeDecoder, _anchorPosSizeModel, _anchorPosModel);
	u_int32_t anchorAdress = CompressionUtils::decodeFixedNumeric(_rangeDecoder, _anchorAdressModel, 4);
	
	//cout << "\tRead size: " << _readSize << endl;
	
	kmer_type anchor = _leon->getAnchor(anchorAdress);
	//anchor = max(anchor, revcomp(anchor, _kmerSize));
	
	if(_rangeDecoder.nextByte(_readAnchorRevcompModel) == 1)
		anchor = revcomp(anchor, _kmerSize);
		
	_currentSeq = anchor.toString(_kmerSize);
	
	/*if((*_leon->_testBankIt)->getIndex() == 197001){
		cout << "\t\t\tAnchor pos: " << anchorPos << endl;
		cout << "\t\t\tAnchor: " << anchor.toString(_kmerSize) << endl;
		cout << "\t\t\t@ Anchor: " << anchorAdress << endl;
	}//#endif*/
	
	_uniqNoOrigMutationPos.clear();
	_Npos.clear();
	
	u_int64_t NposCount = CompressionUtils::decodeNumeric(_rangeDecoder, _anchorPosSizeModel, _anchorPosModel);
	//cout << NposCount << endl;
	for(int i=0; i<NposCount; i++){
		_Npos.push_back(CompressionUtils::decodeNumeric(_rangeDecoder, _anchorPosSizeModel, _anchorPosModel));
	}
	
	
	u_int64_t uniqNoOrigMutation = CompressionUtils::decodeNumeric(_rangeDecoder, _anchorPosSizeModel, _anchorPosModel);
	for(int i=0; i<uniqNoOrigMutation; i++){
		_uniqNoOrigMutationPos.push_back(CompressionUtils::decodeNumeric(_rangeDecoder, _anchorPosSizeModel, _anchorPosModel));
	}
	
	kmer_type kmer = anchor;
	for(int i=anchorPos-1; i>=0; i--){
		kmer = decodeMutations(kmer, i, false);
		//if((*_leon->_testBankIt)->getIndex() == 197001){
		//	cout << "\t" << kmer.toString(_kmerSize) << endl;
		//}
	}
	
	kmer = anchor;
	for(int i=anchorPos+_kmerSize; i<_readSize; i++){
		kmer = decodeMutations(kmer, i, true);
		//cout << "\t" << kmer.toString(_kmerSize) << endl;
	}
	
	
	for(int i=0; i<_Npos.size(); i++){
		_currentSeq[_Npos[i]] = 'N';
	}
}

kmer_type DnaDecoder::decodeMutations(kmer_type kmer, int pos, bool rightExtend){
	
	u_int8_t nextNt;
	kmer_type resultKmer;
		
	if(std::find(_Npos.begin(), _Npos.end(), pos) != _Npos.end()){
		nextNt = 'A';
		if(rightExtend){
			_currentSeq += nextNt;
		}
		else{
			_currentSeq.insert(_currentSeq.begin(), nextNt);
		}
		//cout << _currentSeq << endl;
		//if(nextNt == 'N') nextN
		
		resultKmer = kmer;
		codeSeedNT(&_kmerModel, &resultKmer, nextNt, rightExtend);
		//cout << kmer.toString(_kmerSize) << endl;
		//cout << resultKmer.toString(_kmerSize) << endl;
		return resultKmer;
	}
	
	if(std::find(_uniqNoOrigMutationPos.begin(), _uniqNoOrigMutationPos.end(), pos) != _uniqNoOrigMutationPos.end()){
		nextNt = Leon::bin2nt(_rangeDecoder.nextByte(_mutationModel));
		//cout << "tap 0     " << nextNt << endl;
		//_uniqNoOrigMutationPos.erase(_uniqNoOrigMutationPos.begin());
		if(rightExtend){
			_currentSeq += nextNt;
		}
		else{
			_currentSeq.insert(_currentSeq.begin(), nextNt);
		}
		//cout << _currentSeq << endl;
		//if(nextNt == 'N') nextN
		resultKmer = kmer;
		codeSeedNT(&_kmerModel, &resultKmer, nextNt, rightExtend);
		return resultKmer;
	}
	
		
	//cout << kmer.toString(_kmerSize) << endl;
	kmer_type uniqKmer, mutatedSolidKmer;
	int uniqNt;
	//bool isKmerSolid = false;
	

	//kmer = _kmers[pos];

	int indexedKmerCount = 0;
	
	//cout << kmer.toString(_kmerSize) << endl;
	for(int nt=0; nt<4; nt++){
		//if(nt == original_nt){
		//	continue;
		//}
		
		kmer_type mutatedKmer = kmer;
		codeSeedBin(&_kmerModel, &mutatedKmer, nt, rightExtend);
		kmer_type mutatedKmerMin = min(mutatedKmer, revcomp(mutatedKmer, _kmerSize));
		
		//mutatedKmer.printASCII(_kmerSize);
		
		if(_bloom->contains(mutatedKmerMin)){
			indexedKmerCount += 1;
			uniqNt = nt;
			uniqKmer = mutatedKmer;
		}
		
	}
	
	if(indexedKmerCount == 1){
		nextNt = Leon::bin2nt(uniqNt);
		//cout << "case 1         " << nextNt << endl;
		resultKmer = uniqKmer;
	}
	else{
		nextNt = Leon::bin2nt(_rangeDecoder.nextByte(_mutationModel));
		//cout << "case 2          "<< nextNt << endl;
		resultKmer = kmer;
		codeSeedNT(&_kmerModel, &resultKmer, nextNt, rightExtend);
	}
	
	//cout << nextNt << endl;
	
	if(nextNt == 'N') cout << "lala" << endl;
	
	if(rightExtend){
		_currentSeq += nextNt;
	}
	else{
		_currentSeq.insert(_currentSeq.begin(), nextNt);
	}
	
	//cout << _currentSeq << endl;
	//cout << resultKmer.toString(_kmerSize) << endl;
	return resultKmer;
		
}

void DnaDecoder::decodeNoAnchorRead(){
	#ifdef PRINT_DEBUG_DECODER
		cout << "\t\tDecode no anchor read" << endl;
	#endif
	
	_readSize = CompressionUtils::decodeNumeric(_rangeDecoder, _noAnchorReadSizeModel, _noAnchorReadSizeValueModel);
	//cout << "\tRead size: " << _readSize << endl;
	for(int i=0; i<_readSize; i++){
		_currentSeq += Leon::bin2nt(_rangeDecoder.nextByte(_noAnchorReadModel));
	}
	//endSeq();
	//cout << read << endl;
}
	
void DnaDecoder::endSeq(){
	_currentSeq += '\n';
	_outputFile->write(_currentSeq.c_str(), _currentSeq.size());
	_currentSeq.clear();
}




