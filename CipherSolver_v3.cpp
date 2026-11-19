#include <iostream>
#include <fstream>
#include <chrono>
#include <cmath>
#include <cctype>
#include <algorithm>
#include <random>
#include <limits>
#include <span>
#include <functional>

#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

namespace CipherTools {

	inline std::vector<double> CalcQuadScores(const std::string& filename) {
		std::ifstream file(filename);

		if (!file.is_open()) {
			std::cout << "File " << filename << " failed to open";
			return {};
		}

		std::string quad;
		double score;

		std::vector<double> quadScores(456976, 0);
		double totalScore = 0;

		while (file >> quad >> score) {
			int index = (17576 * (quad[0] - 'A')) + (676 * (quad[1] - 'A')) + (26 * (quad[2] - 'A')) + quad[3] - 'A';

			quadScores[index] = score;
			totalScore += score;
		}

		double quadFloor = std::log10(0.01 / totalScore);
		for (size_t i = 0; i < 456976; ++i) {
			if (quadScores[i] == 0) {
				quadScores[i] = quadFloor;
			}
			else {
				quadScores[i] = std::log10(quadScores[i] / totalScore);
			}
		}

		return quadScores;
	}

	inline double ScoreText(const std::string_view& text, const std::vector<double>& quadScores) {
		if (text.length() < 4) {
			return -10.0;
		}

		double totalScore = 0.0;
		const char* str = text.data();

		for (size_t i = 0; i <= text.length() - 4; ++i) {
			int index = (17576 * (str[i] - 'A')) + (676 * (str[i + 1] - 'A')) + (26 * (str[i + 2] - 'A')) + str[i + 3] - 'A';

			totalScore += quadScores[index];
		}

		return totalScore;
	}

	inline std::string CleanText(const std::string& text) {
		std::string clean = "";
		for (char c : text) {
			if (std::isalpha(c)) {
				clean += std::toupper(c);
			}
		}

		return clean;
	}

	inline std::vector<std::string> LoadWords(const std::string& filename) {
		std::ifstream file(filename);

		if (!file.is_open()) {
			std::cout << "File " << filename << " failed to open";
			return {};
		}

		std::string word;

		std::vector<std::string> words = {};
		while (file >> word) {
			std::string cleanWord = CleanText(word);
			if (cleanWord == "") {
				continue;
			}

			words.push_back(cleanWord);
		}

		return words;
	}

	inline std::vector<std::pair<std::string, int>> CalcFreq(const std::string& text, const size_t chunkSize, const bool print = false) {
		std::unordered_map<std::string, int> counts;

		if (chunkSize == 0 || text.length() < chunkSize) {
			return {};
		}

		for (size_t i = 0; i <= text.length() - chunkSize; i += chunkSize) {
			std::string chunk = text.substr(i, chunkSize);
			counts[chunk]++;
		}

		std::vector<std::pair<std::string, int>> sortedCounts(counts.begin(), counts.end());

		std::sort(sortedCounts.begin(), sortedCounts.end(), [](const auto& a, const auto& b) {
			return a.second > b.second;
			});

		if (print) {
			std::cout << sortedCounts.size() << " chunks analysed\n";

			std::cout << "Frequency: \n";
			for (const auto& [key, value] : sortedCounts) {
				std::cout << key << " : " << value << "\n";
			}
		}

		return sortedCounts;
	}

	inline std::vector<int> TextToNums(const std::string& text) {
		std::vector<int> numText;
		for (const auto& c : text) {
			int index = c - 'A';
			if (index < 0 || index > 25) {
				continue;
			}

			numText.push_back(index);
		}

		return numText;
	}

	inline std::string NumsToText(const std::vector<int>& numText) {
		std::string text;
		for (const auto& num : numText) {
			if (num < 0 || num > 25) {
				continue;
			}

			text.push_back(static_cast<char>('A' + num));
		}

		return text;
	}

	inline double CalcIC(const std::string& text, const size_t offset, const size_t step) {
		int counts[26] = { 0 };
		int totalLetters = 0;
		for (size_t i = offset; i < text.length(); i += step) {
			char c = text[i];
			if (c >= 'A' && c <= 'Z') {
				counts[c - 'A']++;
				totalLetters++;
			}
		}

		if (totalLetters < 2) {
			return 0.0;
		}

		long long numerator = 0;
		for (size_t i = 0; i < 26; ++i) {
			numerator += static_cast<long long>(counts[i]) * (counts[i] - 1);
		}

		double denominator = static_cast<double>(totalLetters) * (totalLetters - 1);

		return static_cast<double>(numerator) / denominator;
	}

	inline int GetIndex(const char c, const char excluded) {
		int index = c - 'A';
		if (c > excluded) index--;
		return index;
	}

	inline std::vector<size_t> WordOrder(const std::string& word) {
		std::vector<std::pair<char, size_t>> chars;
		for (size_t i = 0; i < word.length(); ++i) {
			chars.push_back({ word[i], i });
		}

		std::sort(chars.begin(), chars.end());

		std::vector<size_t> wordOrder(word.length());
		for (size_t i = 0; i < word.length(); ++i) {
			size_t index = chars[i].second;
			wordOrder[index] = i;
		}

		return wordOrder;
	}

}

namespace Random {

	inline double RandDouble(const double min, const double max) {
		static std::random_device rd;
		static std::mt19937 gen(rd());

		std::uniform_real_distribution<double> dist(min, max);

		return dist(gen);
	}

	inline int RandInt(const int min, const int max) {
		static std::random_device rd;
		static std::mt19937 gen(rd());

		std::uniform_int_distribution<int> dist(min, max);

		return dist(gen);
	}

	template <typename T>
	inline T WeightedChoice(const std::vector<T>& choices, const std::vector<double>& weights) {
		if (choices.empty() || choices.size() != weights.size()) {
			std::cout << "There must be at least 1 choice, with the same amount of weights\n";
			return 0;
		}

		static std::random_device rd;
		static std::mt19937 gen(rd());

		std::discrete_distribution<size_t> dist(weights.begin(), weights.end());

		size_t choiceIndex = dist(gen);

		return choices[choiceIndex];
	}

}

namespace VecTools {

	template <typename T>
	inline void ShuffleVec(std::vector<T>& vec) {
		static std::random_device rd;
		static std::mt19937 gen(rd());

		std::shuffle(vec.begin(), vec.end(), gen);
	}

	template <typename T>
	inline std::vector<T> DupeVec(const std::vector<T>& vec, const size_t multiplier) {
		std::vector<T> dupedVec;
		dupedVec.reserve(vec.size() * multiplier);

		for (size_t i = 0; i < multiplier; ++i) {
			dupedVec.insert(dupedVec.end(), vec.begin(), vec.end());
		}

		return dupedVec;
	}

	template <typename T, typename Op>
	inline std::vector<T> ApplyScalar(const std::vector<T>& vec, const T scalar, const Op op) {
		std::vector<T> result;
		result.reserve(vec.size());

		for (const auto& val : vec) {
			result.push_back(op(val, scalar));
		}

		return result;
	}

	template <typename T, typename Op>
	inline std::vector<T> VecOp(const std::vector<T>& vec1, const std::vector<T>& vec2, const Op op) {
		if (vec1.size() != vec2.size()) {
			std::cout << "Vector lengths must be the same\n";
			return {};
		}

		std::vector<T> result;
		result.reserve(vec1.size());

		for (size_t i = 0; i < vec1.size(); ++i) {
			result.push_back(op(vec1[i], vec2[i]));
		}

		return result;
	}

}

namespace Subst {

	struct Param {
		size_t min_cs = 1;
		size_t max_cs = 15;

		double startTemp = 20.0;
		double cooling = 0.999;

		size_t iters = 3000;
		size_t runs = 3;
	};

	struct ReturnData {
		std::string result;
		std::unordered_map<std::string, std::string> mapping;
		
		double score;
		double duration;
	};

	inline std::string Decrypt(const std::string& text, const size_t chunkSize, const std::vector<std::string>& cipherAlphabet, const std::vector<std::string>& alphabet, const bool print = false) {
		if (chunkSize > text.length() || chunkSize == 0 || cipherAlphabet.size() != alphabet.size() || alphabet.empty()) {
			std::cout << "Chunk size must be above 0, text length must be larger than chunk size, cipher alphabet and alphabet size must be identical, and above 0\n";
			return "";
		}

		std::unordered_map<std::string, std::string> mapping;
		for (size_t i = 0; i < cipherAlphabet.size(); ++i) {
			mapping[cipherAlphabet[i]] = alphabet[i];
		}

		std::string decrypted;
		decrypted.reserve(text.length());

		for (size_t i = 0; i + chunkSize <= text.length(); i += chunkSize) {
			std::string chunk = text.substr(i, chunkSize);

			auto it = mapping.find(chunk);
			if (it != mapping.end()) {
				decrypted.append(it->second);
			}
		}

		if (print) {
			std::cout << "Decrypted text: " << decrypted << "\n";
		}

		return decrypted;
	}

	inline ReturnData Auto(const std::string& text, const std::vector<double>& quadScores, const Param& param = {}, const bool print = true) {
		size_t final_cs = 0;
		std::vector<std::string> cipherAlphabet;
		for (size_t cs = param.min_cs; cs < param.max_cs; ++cs) {
			auto freq = CipherTools::CalcFreq(text, cs);
			if (freq.size() >= 20 && freq.size() <= 26) {
				final_cs = cs;

				for (const auto& [key, value] : freq) {
					cipherAlphabet.push_back(key);
				}

				size_t remaining = 26 - cipherAlphabet.size();
				if (remaining > 0) {
					for (size_t i = 0; i < remaining; ++i) {
						cipherAlphabet.push_back("-");
					}
				}
			}
		}

		if (final_cs == 0) {
			std::cout << "No valid chunk size could be found\n";
			return {};
		}

		if (print) {
			std::cout << "Valid chunk size: " << final_cs << "\n";
		}

		std::vector<std::string> freqAlphabet = { "E","T","A","O","I","N","S","R","H","D","L","U","C","M","F","Y","W","G","P","B","V","K","X","Q","J","Z" };

		auto bestKey = freqAlphabet;

		double bestScore = CipherTools::ScoreText(Decrypt(text, final_cs, cipherAlphabet, bestKey), quadScores);

		auto startTime = std::chrono::high_resolution_clock::now();
		for (size_t run = 0; run < param.runs; ++run) {
			if (print) {
				std::cout << "Run " << run + 1 << ", Current best score: " << bestScore << "\n";
			}

			auto currentKey = bestKey;
			VecTools::ShuffleVec(currentKey);

			double currentScore = CipherTools::ScoreText(Decrypt(text, final_cs, cipherAlphabet, currentKey), quadScores);

			double temp = param.startTemp;
			for (size_t i = 0; i < param.iters; ++i) {
				if (i % 1000 == 0 && i > 0 && print) {
					std::cout << "Iteration " << i << ", Overall best: " << bestScore 
						      << ", Temp: " << temp << "\n";
				}

				auto testKey = currentKey;

				int firstIndex = Random::RandInt(0, 25);
				int secondIndex = Random::RandInt(0, 25);

				std::swap(testKey[firstIndex], testKey[secondIndex]);

				double testScore = CipherTools::ScoreText(Decrypt(text, final_cs, cipherAlphabet, testKey), quadScores);
				double delta = testScore - currentScore;

				if (delta > 0.0 || Random::RandDouble(0.0, 1.0) < std::exp(delta / temp)) {
					currentKey = testKey;
					currentScore = testScore;

					if (currentScore > bestScore) {
						bestKey = currentKey;
						bestScore = currentScore;
					}
				}

				temp *= param.cooling;
			}
		}

		auto endTime = std::chrono::high_resolution_clock::now();
		std::chrono::duration<double> duration = endTime - startTime;

		std::unordered_map<std::string, std::string> mapping;
		for (size_t i = 0; i < bestKey.size(); ++i) {
			mapping[bestKey[i]] = cipherAlphabet[i];
		}

		std::vector<std::pair<std::string, std::string>> sortedMapping(mapping.begin(), mapping.end());

		std::sort(sortedMapping.begin(), sortedMapping.end(), [](const auto& a, const auto& b) {
			return a.first < b.first;
			});

		if (print) {
			std::cout << "Best mapping (plaintext : ciphertext): \n";
			for (const auto& [key, value] : sortedMapping) {
				std::cout << key << " : " << value << "\n";
			}

			std::cout << "Iteration time: " << duration.count() << "\n";
		}

		auto result = Decrypt(text, final_cs, cipherAlphabet, bestKey, print);

		ReturnData returnData = { result, mapping, bestScore, duration.count() };

		return returnData;
	}

}

namespace Trans {

	struct Param {
		size_t min_kl = 3;
		size_t max_kl = 12;

		double startTemp = 1000.0;
		double cooling = 0.9995;

		size_t iters = 3000;
		size_t runs = 4;

		std::vector<double> weights = { 100.0, 1.0, 1.0 };
	};

	struct ReturnData {
		std::string result;
		std::vector<size_t> key;

		double score;
		double duration;
	};

	struct DoubleParam {
		size_t min_kl = 3;
		size_t max_kl = 12;

		double startTempPer_kl = 90.0;
		double coolingPer_kl = 0.9995;

		size_t itersPer_kl = 500;

		double startTemp = 610.0;
		double cooling = 0.9996;

		size_t iters = 20000;
		size_t runs = 10;
	};

	struct DoubleReturnData {
		std::string result;
		std::vector<size_t> key1;
		std::vector<size_t> key2;

		double score;
		double duration;
	};

	inline std::vector<size_t> CalcColumnOffsets(const std::vector<size_t>& key, const size_t textLength) {
		std::vector<size_t> columnOffsets(key.size(), 0);

		size_t numChunks = (textLength + key.size() - 1) / key.size();
		size_t remainder = textLength % key.size();
		if (remainder == 0) remainder = key.size();

		size_t currentOffset = 0;
		for (size_t i = 0; i < key.size(); ++i) {
			columnOffsets[key[i]] = currentOffset;

			size_t columnHeight = key[i] < remainder ? numChunks : numChunks - 1;
			currentOffset += columnHeight;
		}

		return columnOffsets;
	}

	inline void Decrypt(const std::string& text, const std::vector<size_t>& key, std::string& out, const std::vector<size_t>& columnOffsets, const bool stan = true, const bool print = false) {
		if (key.empty()) {
			std::cout << "Key cannot be empty\n";
			return;
		}

		size_t numChunks = (text.length() + key.size() - 1) / key.size();

		size_t outIndex = 0;
		if (stan) {
			size_t remainder = text.length() % key.size();
			if (remainder == 0) remainder = key.size();

			for (size_t i = 0; i < numChunks - 1; ++i) {
				for (size_t column = 0; column < key.size(); ++column) {
					size_t index = columnOffsets[column] + i;
					out[outIndex++] = text[index];
				}
			}

			for (size_t column = 0; column < remainder; ++column) {
				size_t index = columnOffsets[column] + numChunks - 1;
				out[outIndex++] = text[index];
			}
		}
		else {
			for (size_t i = 0; i < numChunks; ++i) {
				size_t chunkOffset = i * key.size();

				for (size_t keyIndex : key) {
					size_t targetIndex = chunkOffset + keyIndex;

					if (targetIndex < text.length()) {
						out[outIndex++] = text[targetIndex];
					}
				}
			}
		}

		if (print) {
			std::cout << "Decrypted text: " << out << "\n";
		}
	}

	inline ReturnData Auto(const std::string& text, const std::vector<double>& quadScores, const Param& param = {}, const bool print = true) {
		if (param.weights.size() != 3) {
			std::cout << "Number of weights must be 3\n";
			return {};
		}

		std::vector<size_t> indexes;
		for (size_t i = 0; i < param.max_kl; ++i) {
			indexes.push_back(i);
		}

		std::vector<size_t> bestKey(indexes.begin(), indexes.begin() + param.min_kl);

		bool useStan = true;

		std::string testDecrypt;
		testDecrypt.resize(text.length());
		Decrypt(text, bestKey, testDecrypt, CalcColumnOffsets(bestKey, text.length()), useStan, false);

		double bestScore = CipherTools::ScoreText(testDecrypt, quadScores);
		bool stanBest = true;

		auto startTime = std::chrono::high_resolution_clock::now();
		for (size_t type = 0; type < 2; ++type) {
			if (type == 0 && print) {
				std::cout << "Testing standard columnar transposition\n";
			}
			else if (type == 1 && print) {
				std::cout << "Testing simple transposition\n";
			}

			for (size_t run = 0; run < param.runs; ++run) {
				if (print) {
					std::cout << "Run " << run + 1 << ", Current best score: " << bestScore << "\n";
				}

				auto currentKey = bestKey;
				VecTools::ShuffleVec(currentKey);

				Decrypt(text, currentKey, testDecrypt, CalcColumnOffsets(currentKey, text.length()), useStan, false);

				double currentScore = CipherTools::ScoreText(testDecrypt, quadScores);

				double temp = param.startTemp;
				for (size_t i = 0; i < param.iters; ++i) {
					if (i % 1000 == 0 && i > 0 && print) {
						std::cout << "Iteration " << i << ", Overall best: " << bestScore
							<< ", Temp: " << temp << "\n";
					}

					auto testKey = currentKey;

					std::vector<size_t> choices = { 1, 2, 3 };
					size_t choice = Random::WeightedChoice(choices, param.weights);

					if (choice == 1) {
						int firstIndex = Random::RandInt(0, testKey.size() - 1);
						int secondIndex = Random::RandInt(0, testKey.size() - 1);

						std::swap(testKey[firstIndex], testKey[secondIndex]);
					}
					else if (choice == 2 && testKey.size() < param.max_kl) {
						testKey.push_back(testKey.size());
					}
					else if (choice == 3 && testKey.size() > param.min_kl) {
						size_t toRemove = testKey.size() - 1;

						auto it = std::find(testKey.begin(), testKey.end(), toRemove);
						if (it != testKey.end()) {
							testKey.erase(it);
						}
					}

					Decrypt(text, testKey, testDecrypt, CalcColumnOffsets(testKey, text.length()), useStan, false);

					double testScore = CipherTools::ScoreText(testDecrypt, quadScores);
					double delta = testScore - currentScore;

					if (delta > 0.0 || Random::RandDouble(0.0, 1.0) < std::exp(delta / temp)) {
						currentKey = testKey;
						currentScore = testScore;

						if (currentScore > bestScore) {
							bestKey = currentKey;
							bestScore = currentScore;

							if (type == 1) {
								stanBest = false;
							}
						}
					}

					temp *= param.cooling;
				}
			}

			useStan = false;
		}

		auto endTime = std::chrono::high_resolution_clock::now();
		std::chrono::duration<double> duration = endTime - startTime;

		if (print) {
			if (stanBest) {
				std::cout << "Best decryption method was standard columnar transposition\n";
			}
			else {
				std::cout << "Best decryption method was simple columnar transposition\n";
			}

			std::cout << "Best key:\n";
			for (const auto& keyIndex : bestKey) {
				std::cout << keyIndex << " ";
			}
			std::cout << "\n";

			std::cout << "Iteration time: " << duration.count() << "\n";
		}

		std::string result;
		result.resize(text.length());
		Decrypt(text, bestKey, result, CalcColumnOffsets(bestKey, text.length()), stanBest, print);

		ReturnData returnData = { result, bestKey, bestScore, duration.count() };

		return returnData;
	}

	/*inline DoubleReturnData DoubleAuto(const std::string& text, const std::vector<double>& quadScores, const DoubleParam& param = {}, const bool print = true) {
		size_t snippetLength = std::min(size_t(300), text.length());

		std::vector<size_t> indexes;
		for (size_t i = 0; i < param.max_kl; ++i) {
			indexes.push_back(i);
		}

		std::vector<size_t> bestKey1(indexes.begin(), indexes.begin() + param.min_kl);
		std::vector<size_t> bestKey2(indexes.begin(), indexes.begin() + param.min_kl);
		
		std::string decrypt1;
		std::string decrypt2;
		decrypt1.resize(text.length());
		decrypt2.resize(text.length());

		Decrypt(text, bestKey2, decrypt1, CalcColumnOffsets(bestKey2, text.length()), true, false);
		Decrypt(decrypt1, bestKey1, decrypt2, CalcColumnOffsets(bestKey1, text.length()), true, false);

		double bestScore = CipherTools::ScoreText(decrypt2, quadScores);

		auto startTime = std::chrono::high_resolution_clock::now();
		for (size_t kl1 = param.min_kl; kl1 < param.max_kl + 1; ++kl1) {
			for (size_t kl2 = param.min_kl; kl2 < param.max_kl + 1; ++kl2) {
				std::vector<size_t> currentKey1(indexes.begin(), indexes.begin() + kl1);
				std::vector<size_t> currentKey2(indexes.begin(), indexes.begin() + kl2);

				Decrypt(text, currentKey2, decrypt1, CalcColumnOffsets(currentKey2, text.length()), true, false);
				Decrypt(decrypt1, currentKey1, decrypt2, CalcColumnOffsets(currentKey1, text.length()), true, false);

				double currentScore = CipherTools::ScoreText(decrypt2, quadScores);

				double temp = param.startTempPer_kl;
				for (size_t i = 0; i < param.itersPer_kl; ++i) {
					auto testKey1 = currentKey1;
					auto testKey2 = currentKey2;

					bool useFirst = Random::RandDouble(0.0, 1.0) < 0.5;
					auto useKey = useFirst ? testKey1 : testKey2;

					int firstIndex = Random::RandInt(0, useKey.size() - 1);
					int secondIndex = Random::RandInt(0, useKey.size() - 1);

					std::swap(useKey[firstIndex], useKey[secondIndex]);

					if (useFirst) {
						testKey1 = useKey;
					}
					else {
						testKey2 = useKey;
					}

					Decrypt(text, testKey2, decrypt1, CalcColumnOffsets(testKey2, text.length()), true, false);
					Decrypt(decrypt1, testKey1, decrypt2, CalcColumnOffsets(testKey1, text.length()), true, false);

					double testScore = CipherTools::ScoreText(decrypt2, quadScores);
					double delta = testScore - currentScore;

					if (delta > 0.0 || Random::RandDouble(0.0, 1.0) < std::exp(delta / temp)) {
						currentKey1 = testKey1;
						currentKey2 = testKey2;

						currentScore = testScore;
						if (currentScore > bestScore) {
							bestKey1 = currentKey1;
							bestKey2 = currentKey2;

							bestScore = currentScore;
						}
					}

					temp *= param.coolingPer_kl;
				}
			}
		}

		double temp = param.startTemp;
		for (size_t run = 0; run < param.runs; ++run) {
			if (print) {
				std::cout << "Run " << run + 1 << ", Current best score: " << bestScore << "\n";
			}

			auto currentKey1 = bestKey1;
			auto currentKey2 = bestKey2;

			VecTools::ShuffleVec(currentKey1);
			VecTools::ShuffleVec(currentKey2);

			Decrypt(text, currentKey2, decrypt1, CalcColumnOffsets(currentKey2, text.length()), true, false);
			Decrypt(decrypt1, currentKey1, decrypt2, CalcColumnOffsets(currentKey1, text.length()), true, false);

			std::string_view snippet(decrypt2.data(), snippetLength);
			double currentScore = CipherTools::ScoreText(snippet, quadScores);

			double temp = param.startTemp;
			for (size_t i = 0; i < param.iters; ++i) {
				if (i % 1000 == 0 && i > 0 && print) {
					std::cout << "Iteration " << i << ", Overall best: " << bestScore
						<< ", Temp: " << temp << "\n";
				}

				auto testKey1 = currentKey1;
				auto testKey2 = currentKey2;

				bool useFirst = Random::RandDouble(0.0, 1.0) < 0.5;
				auto useKey = useFirst ? testKey1 : testKey2;

				int firstIndex = Random::RandInt(0, useKey.size() - 1);
				int secondIndex = Random::RandInt(0, useKey.size() - 1);

				std::swap(useKey[firstIndex], useKey[secondIndex]);

				if (useFirst) {
					testKey1 = useKey;
				}
				else {
					testKey2 = useKey;
				}

				Decrypt(text, testKey2, decrypt1, CalcColumnOffsets(testKey2, text.length()), true, false);
				Decrypt(decrypt1, testKey1, decrypt2, CalcColumnOffsets(testKey1, text.length()), true, false);

				std::string_view snippet(decrypt2.data(), snippetLength);
				double testScore = CipherTools::ScoreText(snippet, quadScores);
				double delta = testScore - currentScore;

				if (delta > 0.0 || Random::RandDouble(0.0, 1.0) < std::exp(delta / temp)) {
					currentKey1 = testKey1;
					currentKey2 = testKey2;

					currentScore = testScore;

					double fullScore = CipherTools::ScoreText(decrypt2, quadScores);
					if (fullScore > bestScore) {
						bestKey1 = currentKey1;
						bestKey2 = currentKey2;

						bestScore = fullScore;
					}
				}

				temp *= param.cooling;
			}
		}
		auto endTime = std::chrono::high_resolution_clock::now();

		std::chrono::duration<double> duration = endTime - startTime;
		if (print) {
			std::cout << "Best key 1:\n";
			for (const auto& keyIndex : bestKey1) {
				std::cout << keyIndex << " ";
			}
			std::cout << "\n";

			std::cout << "Best key 2:\n";
			for (const auto& keyIndex : bestKey2) {
				std::cout << keyIndex << " ";
			}
			std::cout << "\n";

			std::cout << "Iteration time: " << duration.count() << "\n";
		}

		Decrypt(text, bestKey2, decrypt1, CalcColumnOffsets(bestKey2, text.length()), true, false);
		Decrypt(decrypt1, bestKey1, decrypt2, CalcColumnOffsets(bestKey1, text.length()), true, false);

		DoubleReturnData returnData = { decrypt2, bestKey1, bestKey2, bestScore, duration.count() };

		return returnData;
	}*/

	inline DoubleReturnData DoubleAuto(const std::string& text, const std::vector<double>& quadScores, const DoubleParam& param = {}, const bool print = true) {
		std::vector<size_t> indexes;
		for (size_t i = 0; i < param.max_kl; ++i) {
			indexes.push_back(i);
		}

		std::vector<size_t> bestKey1(indexes.begin(), indexes.begin() + param.min_kl);
		std::vector<size_t> bestKey2(indexes.begin(), indexes.begin() + param.min_kl);

		std::string decrypt1;
		std::string decrypt2;
		decrypt1.resize(text.length());
		decrypt2.resize(text.length());

		Decrypt(text, bestKey2, decrypt1, CalcColumnOffsets(bestKey2, text.length()), true, false);
		Decrypt(decrypt1, bestKey1, decrypt2, CalcColumnOffsets(bestKey1, text.length()), true, false);

		double bestScore = CipherTools::ScoreText(decrypt2, quadScores);

		auto startTime = std::chrono::high_resolution_clock::now();
		for (size_t kl1 = param.min_kl; kl1 < param.max_kl + 1; ++kl1) {
			for (size_t kl2 = param.min_kl; kl2 < param.max_kl + 1; ++kl2) {
				std::vector<size_t> currentKey1(indexes.begin(), indexes.begin() + kl1);
				std::vector<size_t> currentKey2(indexes.begin(), indexes.begin() + kl2);

				Decrypt(text, currentKey2, decrypt1, CalcColumnOffsets(currentKey2, text.length()), true, false);
				Decrypt(decrypt1, currentKey1, decrypt2, CalcColumnOffsets(currentKey1, text.length()), true, false);

				double currentScore = CipherTools::ScoreText(decrypt2, quadScores);

				double temp = param.startTempPer_kl;
				for (size_t i = 0; i < param.itersPer_kl; ++i) {
					auto testKey1 = currentKey1;
					auto testKey2 = currentKey2;

					bool useFirst = Random::RandDouble(0.0, 1.0) < 0.5;
					auto useKey = useFirst ? testKey1 : testKey2;

					int firstIndex = Random::RandInt(0, useKey.size() - 1);
					int secondIndex = Random::RandInt(0, useKey.size() - 1);

					std::swap(useKey[firstIndex], useKey[secondIndex]);

					if (useFirst) {
						testKey1 = useKey;
					}
					else {
						testKey2 = useKey;
					}

					Decrypt(text, testKey2, decrypt1, CalcColumnOffsets(testKey2, text.length()), true, false);
					Decrypt(decrypt1, testKey1, decrypt2, CalcColumnOffsets(testKey1, text.length()), true, false);

					double testScore = CipherTools::ScoreText(decrypt2, quadScores);
					double delta = testScore - currentScore;

					if (delta > 0.0 || Random::RandDouble(0.0, 1.0) < std::exp(delta / temp)) {
						currentKey1 = testKey1;
						currentKey2 = testKey2;

						currentScore = testScore;
						if (currentScore > bestScore) {
							bestKey1 = currentKey1;
							bestKey2 = currentKey2;

							bestScore = currentScore;
						}
					}

					temp *= param.coolingPer_kl;
				}
			}
		}

		double temp = param.startTemp;
		for (size_t run = 0; run < param.runs; ++run) {
			if (print) {
				std::cout << "Run " << run + 1 << ", Current best score: " << bestScore << "\n";
			}

			auto currentKey1 = bestKey1;
			auto currentKey2 = bestKey2;

			VecTools::ShuffleVec(currentKey1);
			VecTools::ShuffleVec(currentKey2);

			Decrypt(text, currentKey2, decrypt1, CalcColumnOffsets(currentKey2, text.length()), true, false);
			Decrypt(decrypt1, currentKey1, decrypt2, CalcColumnOffsets(currentKey1, text.length()), true, false);

			double currentScore = CipherTools::ScoreText(decrypt2, quadScores);

			double temp = param.startTemp;
			for (size_t i = 0; i < param.iters; ++i) {
				if (i % 1000 == 0 && i > 0 && print) {
					std::cout << "Iteration " << i << ", Overall best: " << bestScore
						<< ", Temp: " << temp << "\n";
				}

				auto testKey1 = currentKey1;
				auto testKey2 = currentKey2;

				bool useFirst = Random::RandDouble(0.0, 1.0) < 0.5;
				auto useKey = useFirst ? testKey1 : testKey2;

				int firstIndex = Random::RandInt(0, useKey.size() - 1);
				int secondIndex = Random::RandInt(0, useKey.size() - 1);

				std::swap(useKey[firstIndex], useKey[secondIndex]);

				if (useFirst) {
					testKey1 = useKey;
				}
				else {
					testKey2 = useKey;
				}

				Decrypt(text, testKey2, decrypt1, CalcColumnOffsets(testKey2, text.length()), true, false);
				Decrypt(decrypt1, testKey1, decrypt2, CalcColumnOffsets(testKey1, text.length()), true, false);

				double testScore = CipherTools::ScoreText(decrypt2, quadScores);
				double delta = testScore - currentScore;

				if (delta > 0.0 || Random::RandDouble(0.0, 1.0) < std::exp(delta / temp)) {
					currentKey1 = testKey1;
					currentKey2 = testKey2;

					currentScore = testScore;
					if (currentScore > bestScore) {
						bestKey1 = currentKey1;
						bestKey2 = currentKey2;

						bestScore = currentScore;
					}
				}

				temp *= param.cooling;
			}
		}
		auto endTime = std::chrono::high_resolution_clock::now();

		std::chrono::duration<double> duration = endTime - startTime;
		if (print) {
			std::cout << "Best key 1:\n";
			for (const auto& keyIndex : bestKey1) {
				std::cout << keyIndex << " ";
			}
			std::cout << "\n";

			std::cout << "Best key 2:\n";
			for (const auto& keyIndex : bestKey2) {
				std::cout << keyIndex << " ";
			}
			std::cout << "\n";

			std::cout << "Iteration time: " << duration.count() << "\n";
		}

		Decrypt(text, bestKey2, decrypt1, CalcColumnOffsets(bestKey2, text.length()), true, false);
		Decrypt(decrypt1, bestKey1, decrypt2, CalcColumnOffsets(bestKey1, text.length()), true, print);

		DoubleReturnData returnData = { decrypt2, bestKey1, bestKey2, bestScore, duration.count() };

		return returnData;
	}

}

namespace Vigen {

	struct ReturnData {
		std::string result;
		std::string key;

		double score;
		double duration;
	};

	namespace Stan {

		struct Param {
			size_t min_kl = 3;
			size_t max_kl = 15;

			double startTemp = 100.0;
			double cooling = 0.9995;

			size_t iters = 5000;
			size_t runs = 1;

			std::vector<double> weights = { 10.0, 20.0, 5.0, 5.0 };
		};

		inline std::string Decrypt(const std::vector<int>& numText, const std::vector<int>& numKey, const bool print = false) {
			if (numText.size() < numKey.size()) {
				std::cout << "Text length should be greater than key length\n";
				return "";
			}

			std::string decrypted;
			decrypted.reserve(numText.size());

			for (size_t i = 0; i < numText.size(); ++i) {
				int modNum = ((numText[i] - numKey[i % numKey.size()]) % 26 + 26) % 26;

				decrypted.push_back(static_cast<char>('A' + modNum));
			}

			if (print) {
				std::cout << "Decrypted text: " << decrypted << "\n";
			}

			return decrypted;
		}

		inline ReturnData Auto(const std::string& text, const std::vector<double>& quadScores, const Param& param = {}, const bool print = true) {
			if (param.weights.size() != 4) {
				std::cout << "Number of weights must be 4\n";
				return {};
			}
			
			std::string clean = CipherTools::CleanText(text);
			auto numText = CipherTools::TextToNums(clean);

			size_t final_kl = 0;
			for (size_t kl = param.min_kl; kl < param.max_kl; ++kl) {
				double total_ic = 0.0;
				for (size_t offset = 0; offset < kl; ++offset) {
					total_ic += CipherTools::CalcIC(clean, offset, kl);
				}

				double av_ic = total_ic / kl;
				if (print) {
					std::cout << "Key length " << kl << ", IC: " << av_ic << "\n";
				}

				if (av_ic > 0.06 && av_ic < 0.07) {
					final_kl = kl;
					break;
				}
			}

			std::vector<int> numAlphabet(26);
			for (int i = 0; i < 26; ++i) {
				numAlphabet[i] = i;
			}

			std::vector<int> bestKey;
			if (final_kl > 0) {
				if (print) {
					std::cout << "The key length " << final_kl << " is likely\n";
				}

				bestKey.assign(numAlphabet.begin(), numAlphabet.begin() + final_kl);
			}
			else {
				if (print) {
					std::cout << "No key length could be found using IC. It will be annealed for\n";
				}

				bestKey.assign(numAlphabet.begin(), numAlphabet.begin() + param.min_kl);
			}

			double bestScore = CipherTools::ScoreText(Decrypt(numText, bestKey), quadScores);

			auto startTime = std::chrono::high_resolution_clock::now();
			for (size_t run = 0; run < param.runs; ++run) {
				if (print) {
					std::cout << "Run " << run << ", Current best score: " << bestScore << "\n";
				}

				auto currentKey = bestKey;
				VecTools::ShuffleVec(currentKey);

				double currentScore = CipherTools::ScoreText(Decrypt(numText, currentKey), quadScores);

				double temp = param.startTemp;
				for (size_t i = 0; i < param.iters; ++i) {
					if (i % 1000 == 0 && i > 0 && print) {
						std::cout << "Iteration " << i << ", Overall best: " << bestScore
							      << ", Temp: " << temp << "\n";
					}

					auto testKey = currentKey;

					std::vector<size_t> choices = { 1, 2, 3, 4 };
					size_t choice = Random::WeightedChoice(choices, param.weights);
					if ((choice == 3 || choice == 4) && final_kl > 0) {
						choice = 2;
					}

					if (choice == 1) {
						int firstIndex = Random::RandInt(0, static_cast<int>(testKey.size() - 1));
						int secondIndex = Random::RandInt(0, static_cast<int>(testKey.size() - 1));

						std::swap(testKey[firstIndex], testKey[secondIndex]);
					}
					else if (choice == 2) {
						int index = Random::RandInt(0, static_cast<int>(testKey.size() - 1));
						int newIndex = Random::RandInt(0, 25);

						testKey[index] = numAlphabet[newIndex];
					}
					else if (choice == 3 && testKey.size() < param.max_kl) {
						int index = Random::RandInt(0, 25);
						testKey.push_back(numAlphabet[index]);
					}
					else if (choice == 4 && testKey.size() > param.min_kl) {
						testKey.pop_back();
					}

					double testScore = CipherTools::ScoreText(Decrypt(numText, testKey), quadScores);
					double delta = testScore - currentScore;

					if (delta > 0.0 || Random::RandDouble(0.0, 1.0) < std::exp(delta / temp)) {
						currentKey = testKey;
						currentScore = testScore;

						if (currentScore > bestScore) {
							bestKey = currentKey;
							bestScore = currentScore;
						}
					}

					temp *= param.cooling;
				}
			}

			auto endTime = std::chrono::high_resolution_clock::now();
			std::chrono::duration<double> duration = endTime - startTime;

			std::string bestTextKey;
			for (const auto& num : bestKey) {
				bestTextKey.push_back(static_cast<char>('A' + num));
			}

			if (print) {
				std::cout << "Best key: " << bestTextKey << "\n";

				std::cout << "Iteration time: " << duration.count() << "\n";
			}

			std::string result = Decrypt(numText, bestKey, print);

			ReturnData returnData = { result, bestTextKey, bestScore, duration.count() };

			return returnData;
		}

	}

	namespace Auto {

		struct Param {
			size_t min_kl = 3;
			size_t max_kl = 15;

			double startTemp = 100.0;
			double cooling = 0.9995;

			size_t iters = 5000;
			size_t runs = 3;

			std::vector<double> weights = { 10.0, 20.0, 5.0, 5.0 };
		};

		inline std::string Decrypt(const std::vector<int>& numText, const std::vector<int>& numKey, const bool print = false) {
			if (numText.size() < numKey.size()) {
				std::cout << "Text length should be greater than key length\n";
				return "";
			}

			std::vector<int> fullKey(numKey.begin(), numKey.end());
			fullKey.reserve(numText.size());

			std::string decrypted;
			decrypted.reserve(numText.size());
			for (size_t i = 0; i < numText.size(); ++i) {
				int modNum = ((numText[i] - fullKey[i]) % 26 + 26) % 26;

				fullKey.push_back(modNum);
				decrypted.push_back(static_cast<char>('A' + modNum));
			}

			if (print) {
				std::cout << "Decrypted text: " << decrypted << "\n";
			}

			return decrypted;
		}

		inline ReturnData Auto(const std::string& text, const std::vector<double>& quadScores, const Param& param = {}, const bool print = true) {
			if (param.weights.size() != 4) {
				std::cout << "Number of weights must be 4\n";
				return {};
			}
			
			std::string clean = CipherTools::CleanText(text);
			auto numText = CipherTools::TextToNums(clean);

			std::vector<int> numAlphabet(26);
			for (int i = 0; i < 26; ++i) {
				numAlphabet[i] = i;
			}

			std::vector<int> bestKey(numAlphabet.begin(), numAlphabet.begin() + param.min_kl);

			double bestScore = CipherTools::ScoreText(Decrypt(numText, bestKey), quadScores);

			auto startTime = std::chrono::high_resolution_clock::now();
			for (size_t run = 0; run < param.runs; ++run) {
				if (print) {
					std::cout << "Run " << run << ", Current best score: " << bestScore << "\n";
				}

				auto currentKey = bestKey;
				VecTools::ShuffleVec(currentKey);

				double currentScore = CipherTools::ScoreText(Decrypt(numText, currentKey), quadScores);

				double temp = param.startTemp;
				for (size_t i = 0; i < param.iters; ++i) {
					if (i % 1000 == 0 && i > 0 && print) {
						std::cout << "Iteration " << i << ", Overall best: " << bestScore
							      << ", Temp: " << temp << "\n";
					}

					auto testKey = currentKey;

					std::vector<size_t> choices = { 1, 2, 3, 4 };

					size_t choice = Random::WeightedChoice(choices, param.weights);
					if (choice == 1) {
						int firstIndex = Random::RandInt(0, static_cast<int>(testKey.size() - 1));
						int secondIndex = Random::RandInt(0, static_cast<int>(testKey.size() - 1));

						std::swap(testKey[firstIndex], testKey[secondIndex]);
					}
					else if (choice == 2) {
						int index = Random::RandInt(0, static_cast<int>(testKey.size() - 1));
						int newIndex = Random::RandInt(0, 25);

						testKey[index] = numAlphabet[newIndex];
					}
					else if (choice == 3 && testKey.size() < param.max_kl) {
						int index = Random::RandInt(0, 25);
						testKey.push_back(numAlphabet[index]);
					}
					else if (choice == 4 && testKey.size() > param.min_kl) {
						testKey.pop_back();
					}
				    
					double testScore = CipherTools::ScoreText(Decrypt(numText, testKey), quadScores);
					double delta = testScore - currentScore;

					if (delta > 0.0 || Random::RandDouble(0.0, 1.0) < std::exp(delta / temp)) {
						currentKey = testKey;
						currentScore = testScore;

						if (currentScore > bestScore) {
							bestKey = currentKey;
							bestScore = currentScore;
						}
					}

					temp *= param.cooling;
				}
			}

			auto endTime = std::chrono::high_resolution_clock::now();
			std::chrono::duration<double> duration = endTime - startTime;

			std::string bestTextKey;
			for (const auto& num : bestKey) {
				bestTextKey.push_back(static_cast<char>('A' + num));
			}

			if (print) {
				std::cout << "Best key: " << bestTextKey << "\n";

				std::cout << "Iteration time: " << duration.count() << "\n";
			}

			std::string result = Decrypt(numText, bestKey, print);

			ReturnData returnData = { result, bestTextKey, bestScore, duration.count() };

			return returnData;
		}

	}

}

namespace Playfair {

	struct Param {
		char excluded = 'J';

		double startTemp = 90.0;
		double cooling = 0.9996;

		size_t iters = 8000;
		size_t runs = 12;

		std::vector<double> weights = { 180.0, 10.0, 10.0, 2.0, 2.0 };
	};

	struct ReturnData {
		std::string result;
		std::vector<char> key;
		uint8_t* lookup;

		double score;
		double duration;
	};

	inline void SwapRows(std::vector<char>& key, const int r1, const int r2) {
		for (int c = 0; c < 5; ++c) {
			std::swap(key[r1 * 5 + c], key[r2 * 5 + c]);
		}
	}

	inline void SwapColumns(std::vector<char>& key, const int c1, const int c2) {
		for (int r = 0; r < 5; ++r) {
			std::swap(key[r * 5 + c1], key[r * 5 + c2]);
		}
	}

	inline void ReverseRow(std::vector<char>& key, const int r) {
		std::swap(key[r * 5], key[r * 5 + 4]);
		std::swap(key[r * 5 + 1], key[r * 5 + 3]);
	}

	inline void ReverseColumn(std::vector<char>& key, const int c) {
		std::swap(key[c], key[20 + c]);
		std::swap(key[5 + c], key[15 + c]);
	}

	inline std::string Decrypt(const std::string& text, const std::vector<char>& key, const uint8_t* lookup, const char excluded = 'J', const bool print = false) {
		auto clean = CipherTools::CleanText(text);
		
		if (clean.length() % 2 != 0 || key.size() != 25) {
			std::cout << "Text length must be even, key length must be 25\n";
			return "";
		}
		
		std::string decrypted;
		decrypted.reserve(clean.length());

		for (size_t i = 0; i < text.length() - 1; i += 2) {
			int pos = lookup[CipherTools::GetIndex(clean[i], excluded)];
			int y1 = pos / 5;
			int x1 = pos % 5;

			pos = lookup[CipherTools::GetIndex(clean[i + 1], excluded)];
			int y2 = pos / 5;
			int x2 = pos % 5;

			char c1;
			char c2;
			if (y1 == y2) {
				c1 = key[(y1 * 5) + (x1 - 1 + 5) % 5];
				c2 = key[(y2 * 5) + (x2 - 1 + 5) % 5];
			}
			else if (x1 == x2) {
				c1 = key[(y1 - 1 + 5) % 5 * 5 + x1];
				c2 = key[(y2 - 1 + 5) % 5 * 5 + x2];
			}
			else {
				c1 = key[(y1 * 5) + x2];
				c2 = key[(y2 * 5) + x1];
			}

			decrypted.push_back(c1);
			decrypted.push_back(c2);
		}

		if (print) {
			std::cout << "Decrypted text: " << decrypted << "\n";
		}

		return decrypted;
	}

	inline ReturnData Auto(const std::string& text, const std::vector<double>& quadScores, const Param& param = {}, const bool print = true) {
		std::string clean = CipherTools::CleanText(text);
		for (char c : clean) {
			if (c == param.excluded) {
				std::cout << "Excluded letter " << param.excluded << " is in the text\n";
				return {};
			}
		}
		
		if (clean.length() % 2 != 0 || param.weights.size() != 5) {
			std::cout << "Text length must be even, number of weights must be 5\n";
			return {};
		}

		uint8_t bestLookup[25];
		std::vector<char> bestKey(25);

		int keyIndex = 0;
		for (size_t i = 0; i < 26; ++i) {
			char current = 'A' + i;
			if (current == param.excluded) continue;

			bestKey[keyIndex] = current;
			bestLookup[CipherTools::GetIndex(current, param.excluded)] = keyIndex;
			keyIndex++;
		}

		double bestScore = CipherTools::ScoreText(Decrypt(clean, bestKey, bestLookup, param.excluded), quadScores);

		auto startTime = std::chrono::high_resolution_clock::now();
		for (size_t run = 0; run < param.runs; ++run) {
			if (print) {
				std::cout << "Run " << run + 1 << ", Current best score: " << bestScore << "\n";
			}

			auto currentKey = bestKey;
			VecTools::ShuffleVec(currentKey);

			uint8_t currentLookup[25];
			for (int i = 0; i < 25; ++i) {
				currentLookup[CipherTools::GetIndex(currentKey[i], param.excluded)] = i;
			}

			double currentScore = CipherTools::ScoreText(Decrypt(clean, currentKey, currentLookup, param.excluded), quadScores);

			double temp = param.startTemp;
			for (size_t i = 0; i < param.iters; ++i) {
				if (i % 1000 == 0 && i > 0 && print) {
					std::cout << "Iteration " << i << ", Overall best: " << bestScore
						      << ", Temp: " << temp << "\n";
				}

				auto testKey = currentKey;
				uint8_t testLookup[25];

				std::vector<int> choices = { 1, 2, 3, 4, 5 };

				int choice = Random::WeightedChoice(choices, param.weights);

				if (choice == 1) {
					int firstIndex = Random::RandInt(0, 24);
					int secondIndex = Random::RandInt(0, 24);

					std::swap(testKey[firstIndex], testKey[secondIndex]);
				}
				else if (choice == 2) {
					int r1 = Random::RandInt(0, 4);
					int r2 = Random::RandInt(0, 4);

					SwapRows(testKey, r1, r2);
				}
				else if (choice == 3) {
					int c1 = Random::RandInt(0, 4);
					int c2 = Random::RandInt(0, 4);

					SwapColumns(testKey, c1, c2);
				}
				else if (choice == 4) {
					int r = Random::RandInt(0, 4);
					
					ReverseRow(testKey, r);
				}
				else {
					int c = Random::RandInt(0, 4);

					ReverseColumn(testKey, c);
				}

				for (int i = 0; i < 25; ++i) {
					testLookup[CipherTools::GetIndex(testKey[i], param.excluded)] = i;
				}

				double testScore = CipherTools::ScoreText(Decrypt(clean, testKey, testLookup, param.excluded), quadScores);
				double delta = testScore - currentScore;

				if (delta > 0.0 || Random::RandDouble(0.0, 1.0) < std::exp(delta / temp)) {
					currentKey = testKey;
					currentScore = testScore;

					if (currentScore > bestScore) {
						bestKey = currentKey;
						bestScore = currentScore;
					}
				}

				temp *= param.cooling;
			}
		}

		auto endTime = std::chrono::high_resolution_clock::now();
		std::chrono::duration<double> duration = endTime - startTime;

		if (print) {
			std::cout << "Best grid: ";
			for (size_t i = 0; i < 25; ++i) {
				if (i % 5 == 0) {
					std::cout << "\n";
				}

				std::cout << bestKey[i] << " ";
			}
			std::cout << "\n";

			std::cout << "Iteration time: " << duration.count() << "\n";
		}

		for (int i = 0; i < 25; ++i) {
			bestLookup[CipherTools::GetIndex(bestKey[i], param.excluded)] = i;
		}

		std::string result = Decrypt(clean, bestKey, bestLookup, param.excluded, print);

		ReturnData returnData = { result, bestKey, bestLookup, bestScore, duration.count() };

		return returnData;
	}

}

namespace Bifid {

	struct Param {
		char excluded = 'J';

		size_t minPeriod = 2;
		size_t maxPeriod = 15;

		double startTemp = 90.0;
		double cooling = 0.9996;

		size_t iters = 5000;
		size_t runs = 4;

		std::vector<double> weights = { 100.0, 1.0, 1.0, 1.0, 1.0 };
	};

	struct ReturnData {
		std::string result;
		std::vector<char> key;
		uint8_t* lookup;

		double score;
		double duration;
	};

	inline std::string Decrypt(const std::string& text, const std::vector<char>& key, const uint8_t* lookup, const size_t period, const char excluded = 'J', const bool print = false) {
		auto clean = CipherTools::CleanText(text);

		if (key.size() != 25) {
			std::cout << "Key length must be 25\n";
			return "";
		}

		std::string decrypted;
		decrypted.reserve(clean.length());

		std::vector<uint8_t> fullPeriod(period * 2);

		std::vector<uint8_t> lastPeriod(clean.length() % period);
		int lastIndex = clean.length() / period;
		
		bool last = false;
		for (size_t i = 0; i < (clean.length() + period) / period; ++i) {
			if (last) {
				break;
			}

			for (size_t j = 0; j < period; ++j) {
				if (i * period + j > clean.length() - 1) {
					last = true;
					break;
				}

				int pos = lookup[CipherTools::GetIndex(clean[i * period + j], excluded)];

				fullPeriod[j * 2] = pos / 5;
				fullPeriod[j * 2 + 1] = pos % 5;
			}

			int usePeriod = last ? clean.length() % period : period;
			for (size_t j = 0; j < usePeriod; ++j) {
				int y = fullPeriod[j];
				int x = fullPeriod[j + usePeriod];

				decrypted.push_back(key[y * 5 + x]);
			}
		}

		if (print) {
			std::cout << "Decrypted text: " << decrypted << "\n";
		}

		return decrypted;
	}

	inline ReturnData Auto(const std::string& text, const std::vector<double>& quadScores, const Param& param = {}, const bool print = true) {
		std::string clean = CipherTools::CleanText(text);
		for (char c : clean) {
			if (c == param.excluded) {
				std::cout << "Excluded letter " << param.excluded << " is in the text\n";
				return {};
			}
		}

		if (param.weights.size() != 5) {
			std::cout << "Number of weights must be 5\n";
			return {};
		}

		uint8_t bestLookup[25];
		std::vector<char> bestKey(25);

		int keyIndex = 0;
		for (size_t i = 0; i < 26; ++i) {
			char current = 'A' + i;
			if (current == param.excluded) continue;

			bestKey[keyIndex] = current;
			bestLookup[CipherTools::GetIndex(current, param.excluded)] = keyIndex;
			keyIndex++;
		}

		size_t bestPeriod = param.minPeriod;

		double bestScore = CipherTools::ScoreText(Decrypt(clean, bestKey, bestLookup, bestPeriod, param.excluded), quadScores);

		auto startTime = std::chrono::high_resolution_clock::now();
		for (size_t run = 0; run < param.runs; ++run) {
			if (print) {
				std::cout << "Run " << run + 1 << ", Current best score: " << bestScore << "\n";
			}

			auto currentKey = bestKey;
			VecTools::ShuffleVec(currentKey);

			uint8_t currentLookup[25];
			for (int i = 0; i < 25; ++i) {
				currentLookup[CipherTools::GetIndex(currentKey[i], param.excluded)] = i;
			}

			size_t currentPeriod = bestPeriod;

			double currentScore = CipherTools::ScoreText(Decrypt(clean, currentKey, currentLookup, currentPeriod, param.excluded), quadScores);

			double temp = param.startTemp;
			for (size_t i = 0; i < param.iters; ++i) {
				if (i % 1000 == 0 && i > 0 && print) {
					std::cout << "Iteration " << i << ", Overall best: " << bestScore
				              << ", Temp: " << temp << "\n";
				}

				auto testKey = currentKey;
				uint8_t testLookup[25];

				size_t testPeriod = currentPeriod;

				std::vector<int> choices = { 1, 2, 3, 4, 5 };
				int choice = Random::WeightedChoice(choices, param.weights);
				if (testPeriod == clean.length() && (choice == 2 || choice == 3)) {
					choice = 1;
				}


				if (choice == 1) {
					int firstIndex = Random::RandInt(0, 24);
					int secondIndex = Random::RandInt(0, 24);

					std::swap(testKey[firstIndex], testKey[secondIndex]);
				}
				else if (choice == 2 && testPeriod < param.maxPeriod) {
					testPeriod++;
				}
				else if (choice == 3 && testPeriod > param.minPeriod) {
					testPeriod--;
				}
				else if (choice == 4) {
					testPeriod = clean.length();
				}
				else if (choice == 5) {
					testPeriod = param.minPeriod;
				}

				for (int j = 0; j < 25; ++j) {
					testLookup[CipherTools::GetIndex(testKey[j], param.excluded)] = j;
				}

				double testScore = CipherTools::ScoreText(Decrypt(clean, testKey, testLookup, testPeriod, param.excluded), quadScores);
				double delta = testScore - currentScore;

				if (delta > 0.0 || Random::RandDouble(0.0, 1.0) < std::exp(delta / temp)) {
					currentKey = testKey;
					currentPeriod = testPeriod;
					currentScore = testScore;

					if (currentScore > bestScore) {
						bestKey = currentKey;
						bestPeriod = currentPeriod;
						bestScore = currentScore;
					}
				}

				temp *= param.cooling;
			}
		}

		auto endTime = std::chrono::high_resolution_clock::now();
		std::chrono::duration<double> duration = endTime - startTime;

		if (print) {
			if (bestPeriod == clean.length()) {
				std::cout << "No period was used\n";
			}
			else {
				std::cout << "Best period: " << bestPeriod << "\n";
			}

			std::cout << "Best grid: ";
			for (size_t i = 0; i < 25; ++i) {
				if (i % 5 == 0) {
					std::cout << "\n";
				}

				std::cout << bestKey[i] << " ";
			}
			std::cout << "\n";

			std::cout << "Iteration time: " << duration.count() << "\n";
		}

		for (int i = 0; i < 25; ++i) {
			bestLookup[CipherTools::GetIndex(bestKey[i], param.excluded)] = i;
		}

		std::string result = Decrypt(clean, bestKey, bestLookup, bestPeriod, param.excluded, print);

		ReturnData returnData = { result, bestKey, bestLookup, bestScore, duration.count() };

		return returnData;
	}

}

namespace Program {

	inline void ClearConsole() {
		std::cin.clear();
		std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
	}

	template <typename T>
	inline T GetValidNum(const std::string& prompt, const T minVal, const T maxVal) {
		T value{};
		while (true) {
			std::cout << prompt;

			if (!(std::cin >> value)) {
				std::cout << "That is not an option, try again\n";
				ClearConsole();
			}
			else if (value < minVal || value > maxVal) {
				std::cout << "That is not an option, try again\n";
			}
			else {
				std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
				return value;
			}
		}
	}

	inline bool YesNoChoice(const std::string& prompt) {
		std::string option;
		while (true) {
			std::cout << prompt;
			std::getline(std::cin, option);

			option = CipherTools::CleanText(option);
			if (option == "Y") {
				return true;
			}
			else if (option == "N") {
				return false;
			}
			else {
				std::cout << "That is not an option, try again\n";
			}
		}
	}

	inline char GetExcluded(const std::string& text, const std::string& prompt) {
		auto freq = CipherTools::CalcFreq(text, 1, false);

		std::string excluded_str;
		std::cout << prompt;
		std::getline(std::cin, excluded_str);

		char excluded;
		if (excluded_str.length() != 1) {
			std::cout << "That is not an option, try again\n";
			excluded = GetExcluded(text, prompt);
			return excluded;
		}
		else {
			excluded = std::toupper(excluded_str[0]);
			if (excluded < 'A' || excluded > 'Z') {
				std::cout << "Excluded character must be a letter, try again\n";
				excluded = GetExcluded(text, prompt);
				return excluded;
			}
		}

		for (const auto& item : freq) {
			if (item.first[0] == excluded) {
				std::cout << "That letter is in the text, try again\n";
				excluded = GetExcluded(text, prompt);
				return excluded;
			}
		}

		return excluded;
	}

	inline void Run() {
		std::string text;
		std::cout << "Please enter text: ";
		std::getline(std::cin, text);

		std::string prompt = "Would you like to clean the text (Y/N)?\n";
		bool yes = YesNoChoice(prompt);

		if (yes) {
			text = CipherTools::CleanText(text);
			std::cout << "The text has been cleaned\n";
		}
		else {
			std::cout << "The text won't be cleaned\n";
		}

		int textLength = text.length();

		auto quadScores = CipherTools::CalcQuadScores("english_quadgrams.txt");

		while (true) {
			std::cout << "These are the options:\n";

			int largestNum = 7;
			std::cout << "1. Frequency analysis\n";
			std::cout << "2. Automatic Substitution solver\n";
			std::cout << "3. Automatic Transposition solver\n";
			std::cout << "4. Automatic Standard Vigenere solver\n";
			std::cout << "5. Automatic Autokey Vigenere solver\n";
			std::cout << "6. Automatic Playfair solver\n";
			std::cout << "7. Automatic Bifid solver\n";

			prompt = "What option would you like to pick?\n";
			int num = GetValidNum(prompt, 1, largestNum);

			if (num == 1) {
				prompt = "Frequency analysis requires a chunk size. What chunk size would you like to test?\n";
				int cs = GetValidNum(prompt, 1, textLength);

				CipherTools::CalcFreq(text, cs, true);
			}
			else if (num == 2) {
				std::cout << "Using default settings\n";
				Subst::Auto(text, quadScores);
			}
			else if (num == 3) {
				std::cout << "Using default settings\n";
				Trans::Auto(text, quadScores);
			}
			else if (num == 4) {
				auto clean = CipherTools::CleanText(text);

				std::cout << "Using default settings\n";
				Vigen::Stan::Auto(clean, quadScores);
			}
			else if (num == 5) {
				auto clean = CipherTools::CleanText(text);

				std::cout << "Using default settings\n";
				Vigen::Auto::Auto(clean, quadScores);
			}
			else if (num == 6) {
				auto clean = CipherTools::CleanText(text);

				std::cout << "Playfair operates on a 5x5 grid, meaning one letter must be excluded from the grid - this letter cannot be in the ciphertext\n";
				prompt = "What letter would you like to be excluded?\n";

				char excluded = GetExcluded(clean, prompt);

				Playfair::Auto(clean, quadScores, { excluded = excluded });
			}
			else if (num == 7) {
				auto clean = CipherTools::CleanText(text);

				std::cout << "Bifid operates on a 5x5 grid, meaning one letter must be excluded from the grid - this letter cannot be in the ciphertext\n";
				prompt = "What letter would you like to be excluded?\n";

				char excluded = GetExcluded(clean, prompt);
				
				Bifid::Auto(clean, quadScores, { excluded = excluded });
			}

			prompt = "Would you like to continue with more options (Y/N)?\n";

			bool cont = YesNoChoice(prompt);

			if (!cont) {
				break;
			}
		}
	}

}

int main() {
	Program::Run();
	/*auto quadScores = CipherTools::CalcQuadScores("english_quadgrams.txt");

	std::string text = "VTWDRLOEHSWYASHELNAUTAASUCPHBODTIRCIQUAYNWCOBIMAUSENENESDNHNYSTMTEAHTESNNMNUTSOICASHEAESANRNECHIHATWTALRDEDMEGHSTIOOROAPAWNNEPRISCADNOALIKANOMNBDTDEOGRBENTESYNWETTLETRINECAIOSCAFNNESMOETRHHGNLMNSOPNLSDUEIHFESSICNDIRNMOYTUOLEYRHDAESOIOOASAGTUNCEOOSPFNURIAOTYETUNRNSETSSEEURRTRDRMRPDONNMEAHRSERTWCIORSLESROIEEFHGODNUEHSTSDALEHIHOHFPACIEFFBAUSVEDNMUNNRRUNAHRDESORCIRTRODBUHVIHRCECEROPAALGEIITPOIGLGUCOYMVAITDNPAHNFCAECRMUSRIOEHBYGDCTSACEUVDINEWOOANENPAOENWIEMESEOLTECXARISSARWALAEDEINNHTDDNTNNSAIONHEOAPCVNSSATSESELRROREFOYLEAECEETPOIOOIAAIMVAODOYYOSYTBTEIDIUBTKHYTMUONSNTTFRIOIIELDITCPYGTLAGAKHOVYEHHNKNKCNEYEORTEAPYHKOGCTTSETECEWITETUAETHHVSESADSRHNGFEOORENEBESEOUSROMHTOYSEGITWTTIRPTSRTEYUYCSEHAHETUEATPASSIGEUTDINNRUNTEAVEINATDNDNNUDOMEHPERHCHQYRVDEOEEINTTFTPEDERLGNEETEUNEANEEDNAIFTAHTENOIOHNARHUMTGSEAHGEIOIUTINHRNUNNNMOSETIEICIYTGTMRSWKADNDEHNATNPRTIVHNEASEOGRSEUSTEIEITVHDTUREITRGANIICITMOTDTEDERMHNTDSOYAAEOASSOETNTNEIHRPMEOEARNOILRAAVEEEDEICKLNSLHGSMARPLOEOLETNEUERTFOYDRMOHNSAMCNGNSTMEEBTIRINNBTOUYNEISGEYIATIURSTKEHDRIEOIFAOLITWNEDNTEHHRLDWTANATIRRDNGESATBRAREASOEOAIKRBAEAWCETETDCCANNSAAIONFAVAEGIEHGNIPWHTIEAUREEEPNTHEEMHNEFETRTKMRNFITMLSHHIDIIATIIDISAERDPMATEOFIWOKHRARHLYAFITTTOAOSJIATUFEIEEUVDEIMCNSANEETBIIDTEHOMIAIHNUHTEEYUREVMUOMHTUGLVYBPLENIISGNSIGORYIAEIMLNTIDNERABNAAAIIOPRCEEOYRABWIEHUOHSHEIENEFMIMUOYNEGNTIEIOADHDRASAORADNBEEHFILGUAEFMDNNUTTEURMWDCITMIENASOATASMLDMOIMDYCTREEDIEIWHNADTNTYIEIEOITAVDRABMNMIRNBACDNEYEOCEITOOEHHIHEPIMITMAEERHXEMOHANDRTBAYTMNEGMAEMNVHDUPMRLIBITEGEPSURNOOIHAHQHCTENMWSERTLTAAPEAREAROLTEIYODIKERTVGRHNUICFITWDOEMARHSNRMMLRDAATLEDOSOACLUNOTEAREEDEESEIYLEEAVTENIRNCHHCWTTTMEKIUAOIVTEWEBNTTHNRUNLMNNNOYOHARDAHINAIDEIAGTTFEORGRTADREODDMEIUHRTODVTRHEESSHACIPSMSEIOEMTFTIPEWAIMBNNBHNNASTSLKTHESIEFIMMBEEINDAAWSCSAIAFNUSEUCATESIRHDIPEEOONRBROWTNFITGTMIIALIHTSKTHHLCFLPNSEHHNTEYSNNHSOEFICPEIXPIOEHMGNNHALTILNAUIHDPICLARIFOLSTIURAOEHGALEKCDOTHEUSYAONHASOITITGNHMEANKTHDDEDTOOSEVHAFIGOICTYTOOAELDNEHRXENOOBSEPSNTEOTTTENIANEITLTAYLNEDVANRFMIADSILNAEHIMRROHTIDIPCAKTSAGNORCTPALNEAMIRODEGAINNLMASRGEDSASRNLVAOACRIAANMCSNIYTMHRAQHENSRIIHARDEATIEFOANRCFBAATSEEEIECUIOVTLTYIEAUITITRMLBVSIIUEORTNEYNIENODTDASTSIADYCUAFAOAEMTTIGTAEFTEUVERHDAEOCNIERWGBTESIRDTEBEHOAXMPNEYSENPDLACMYOYRTSYTATIIIIPNIHESFNSONIROCSWDEEUCVIETNQNCLTEETTOIROTIUIGVWRESYNOEEDRSCCIRDESBNNUTPAOEAGOWIRRTVEAMCRNNUOEHAPNIEKEIHIASUATONTVOLOATKCSGISGOADISHSETLEEEHHGUEOYHRTOSUNTNHTPNAEOACVOG";
	text = CipherTools::CleanText(text);

	std::string test = "IRINBFAENEANDAAIWIMETHIUTIVMEHUDIYTRVOIEIIHAEEEEARBIIYAOFSNRGRMATDINGCANLDOLVCPHDOTEUEEIMATESECISEOIESOMUENVTASNSCNVAGDEEBPEEROUFDITREMTEDGIAIAHLALYANHUHHRFERKASSSCAAGEORNOHNTRTLTONDTNIIOPWETAGTENEIEYMIDBNOIOONSDTOPCEETSGSTTNIRGEBROVISAOIOYAILORIROPIMUKSCENNNTTHDDEDTCGBLEWFCSAOIFDTNWERSUIYRNAVBIDARNIRMNTITGOOYARNOSAKWNKENDNENGRAHNPHOUTTEISEFSIHFERRNSGNIDOIAEOPDETYECDNTGEIBSRUHVSRLSVANWAETIAACEADONMRPIELSLEIYEEECIGIOOTWETMOARNUVEOLEEHUEOEROETSXEAHAUGTENATTNTDUMAEHPEOTCBCTGENTNNVMEYAYTERTTISHCHSARIITAICERAAEMPISRRNDNEWAHTTVCYSSNADYDEOHHDEUOIDENEIRTWTDYKHRKTDHITONCERYNTCDREOTHNAHICIOWHHMNINRERSCWFMHMTVMAASNOTETOPHVMAERACTACEAALYUCASDTEAIUTNFHEURNNTUOOATISTNEUVUAFTEOSYRMRCTSSBRCHRDDYTBTYACGCEADAMEAASDHAHTDITMNGTIIEIACATEPANRDAGTLELSNFRUOEWHSMRFRUNAHCLRMTHHSTPEIHEIBSECRHVIFRNSTRNBEASENOEFTHBENEIHOCSAEOERSTROTOEHLLSRRNEKAETOAASAHOHISMNRIITANDHRTSERIOPNIEEENIMOCTAAMNNIVTCOODKTVOAEITDINMATOEUDHHOCIUOODDEGOPILCKBFKEEAYUEESDRGSCIAIOREHEUTCSICHONHIEPASTTRMFTDNNEUEAMNUDABOOWETYIHOOHAENDEICHAOEEILSSTEWWHHASNOSSOHNILNARAEOIHRTSHRNTAOIBSRPTEOALITEGTITIDDPITDBAEEARONEMOVOESHRILYAIFHNPROITAKUIFIINGNWEENECRRITENIOMPBINTYHTIVMNAHNGODPENHEUTAOEEEUENTORAHALITCNGGMTEETMIAROMIDUATTAHILHESOTCOOUTAEOOSDIPHSRERHOREYLITVHYHNHITGNAISEOEFSVWMEFEEORENTIALSFNUNHHEWISHDLEHNUITINALEXHFIEKYETCMVPOLRERITSOMSAHDYTNLSCIMISRTHIISRNDQEEMPTAOOAAAEOKEEOEBLWNPYAEWCNRAHTBUSEEESTVEDAWHYNNATITODSLIEKLFTHNUNISERAWIAOLEUCROPIGIRAITAYMHOEMIEEEITIRNNEIERLEIAPTTNNIFMHREESPLANEEIPSAFTHILROURMOYNRCANWIOEDUCSGIBAPTNRNGASIRIMNEWADLEOQIISWGTNACANSWNERIOEAGHEOAODAILOENAASEISOENLSRISCNROBRSACSANETTSSYNAUAUMETEXIODEHUPTHPAINETOENQBLTTUTCHYTMNREYCOLANSEUILHGSOHTEAIEDDEGEEOWOYIHANKEEDSSOSOHACAIYIHUETVEUSATKTHTHOYSOAEOOWIENFNIENAWPSRUNUFNYOPIDSREMNAMNUTAEECTOHNINEWTYANEMDIRENUUDHTLEETSTGRINCSUUEHHIIAADFIMRTTDOEIAGEHDREEDDTUYSEVNLILPAMGAHOYOAANINGECARTTOTNNIHIGIDPIGETEGQUEYMIARERIOFEGGVELNVEHKMEYCCHPSOHGEIEHSTHCNEEECMODSRMIOTNNTLELAROAEAHBIRITHTEEDRRSHUDEPIHEAEXGNEREMICHNNIHMINPHLNROSOEMAIANARRYIRENMTSTTIRUAOIOEEINDLGMSMSCMBFNNDDNMNONESERRUNEMTDPTNELDAUTOFERDAETSDVTNVTLMGNREMAYSTLEAONOMDNMSEEOYIRODTBDASSSOEOSAKLASAMIHRCORRANRDEPALTATIVTTTTADGNTIEIETIDETVIHIHEOAAOFEDNEMIDEANOTKTTLITUEYUTEYHIYDFTPASFUNSAEROITQUSRHEASWMREIOHEDMDYSTTEHNURERMTGNODNMDFDITKRRUUSELAEEMCTTBFWUTNTEHSAEUTYFSEPRINJTEXONTNMNNBBALDFEHCEGOACALLESIEATANVNDSTITRHINRNFGDNMTVVATSESEIDCSAINAPENIPRGLRNCCCRRUAIELOYTENOONUEABPOSNEYHUBKEMBABIGTINAPYA";
	test = CipherTools::CleanText(test);*/

	/*auto key1 = CipherTools::WordOrder("CATACLYSMIC");
	auto key2 = CipherTools::WordOrder("DESTRUCTIONS");

	std::vector<size_t> inv1(key1.size());
	std::vector<size_t> inv2(key2.size());

	for (int i = 0; i < key1.size(); ++i) {
		inv1[key1[i]] = i;
	}

	for (int i = 0; i < key2.size(); ++i) {
		inv2[key2[i]] = i;
	}

	auto start = std::chrono::high_resolution_clock::now();

	std::string first;
	first.resize(test.length());

	std::string second;
	second.resize(test.length());

	auto co1 = Trans::CalcColumnOffsets(inv1, test.length());
	auto co2 = Trans::CalcColumnOffsets(inv2, text.length());

	for (int i = 0; i < 100000; ++i) {
		Trans::Decrypt(test, inv2, first, co2, true, false);
		Trans::Decrypt(first, inv1, second, co1, true, false);
	}
	auto end = std::chrono::high_resolution_clock::now();

	std::chrono::duration<double> duration = end - start;
	std::cout << duration.count();*/

    /*int tally = 0;
	int ftally = 0;
	int stally = 0;
	for (int i = 0; i < 10; ++i) {
		auto data = Trans::DoubleAuto(test, quadScores);

		if (data.score > -10057) {
			tally++;
		}
		else {
			if (data.key1.size() != 11 || data.key2.size() != 12) {
				ftally++;
			}
			else {
				stally++;
			}
		}
	}

	std::cout << tally << "\n";
	std::cout << ftally << "\n";
	std::cout << stally << "\n";*/

	/*double minTemp = 10.0;
	double maxTemp = 1000.0;
	double tempStep = 10.0;

	double minCooling = 0.999;
	double maxCooling = 0.99999;
	double coolingStep = 0.0001;

	Trans::DoubleParam currentParam = { 6, 13, 90.0, 0.9995, 500, minTemp, minCooling };
	std::vector<Trans::DoubleParam> validParam = {};
	
	while (currentParam.startTemp < maxTemp) {
		std::cout << "Testing start temp " << currentParam.startTemp << "\n";

		while (currentParam.cooling < maxCooling) {
			std::cout << "Testing cooling " << currentParam.cooling << "\n";

			int score = 0;
			for (int i = 0; i < 3; ++i) {
				auto data = Trans::DoubleAuto(test, quadScores, currentParam, false);

				if (data.score > -10057) {
					score++;
				}
			}

			if (score == 3) {
				std::cout << "Valid param found - Start temp: " << currentParam.startTemp << ", Cooling: " << currentParam.cooling << "\n";

				validParam.push_back(currentParam);
			}

			currentParam.cooling += coolingStep;
		}

		currentParam.cooling = minCooling;
		currentParam.startTemp += tempStep;
	}

	std::cout << "Beginning final param test\n";
	std::cout << "Final valid param:\n";

	std::vector<Trans::DoubleParam> finalParam = {};
	for (const auto& param : validParam) {
		int score = 0;
		for (int i = 0; i < 20; ++i) {
			auto data = Trans::DoubleAuto(test, quadScores, param, false);

			if (data.score > -10057) {
				score++;
			}
		}

		std::cout << "Start Temp: " << param.startTemp << ", Cooling: " << param.cooling << " - Score: " << score << "\n";
		if (score > 10) {
			finalParam.push_back(param);
		}
	}

	for (const auto& param : finalParam) {
		int score = 0;
		for (int i = 0; i < 100; ++i) {
			auto data = Trans::DoubleAuto(test, quadScores, param, false);

			if (data.score > -10057) {
				score++;
			}
		}

		std::cout << "Start Temp: " << param.startTempPer_kl << ", Cooling: " << param.coolingPer_kl << " - Score: " << score << "\n";
	}*/

	return 0;
}