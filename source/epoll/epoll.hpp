/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   epoll.hpp                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: nacao <nacao@student.42.fr>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/08/06 14:53:33 by naiqing           #+#    #+#             */
/*   Updated: 2025/08/07 16:25:30 by nacao            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef EPOLL_HPP
#define EPOLL_HPP

#include "../../include/webserv.hpp"

class Epoll
{
    private:
        int _epollfd; // File descriptor for the epoll instance

    public:
        int initEpoll(Socket &socket);;
        Epoll(/* args */);
        ~Epoll();
};


#endif