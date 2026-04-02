/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   CGIUtils.cpp                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: tissad <tissad@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/04/10 14:20:52 by tissad            #+#    #+#             */
/*   Updated: 2025/04/12 19:36:16 by tissad           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */


#include "CGIUtils.hpp"

// std ::vector<std::string> splitString(const std::string& str, char delimiter)
// {
// 	std::vector<std::string> result;
// 	std::string token;
// 	size_t start = 0;
// 	size_t end = str.find(delimiter);

// 	while (end != std::string::npos) {
// 		token = str.substr(start, end - start);
// 		result.push_back(token);
// 		start = end + 1;
// 		end = str.find(delimiter, start);
// 	}
// 	if (start < str.length()) {
// 		result.push_back(str.substr(start));
// 	}
// 	return result;
// }