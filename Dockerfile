FROM ubuntu
MAINTAINER Marco Magliano

ARG ARG_ACCOUNT_GMAIL_TO
ARG ARG_ACCOUNT_GMAIL_USER
ARG ARG_ACCOUNT_GMAIL_PASSWORD
ARG ARG_GIT_BRANCH=master


ENV ACCOUNT_GMAIL_TO=$ARG_ACCOUNT_GMAIL_TO
ENV ACCOUNT_GMAIL_USER=$ARG_ACCOUNT_GMAIL_USER
ENV ACCOUNT_GMAIL_PASSWORD=$ARG_ACCOUNT_GMAIL_PASSWORD
ENV GIT_BRANCH=$ARG_GIT_BRANCH

RUN useradd -rm -d /home/clifueluser -s /bin/bash -u 600 clifueluser

WORKDIR /home/clifueluser

RUN apt-get update
RUN apt-get install -y gcc make git wget unzip libcurl4-openssl-dev libncurses5-dev libssl-dev
RUN git clone https://github.com/muquit/mailsend.git
WORKDIR /home/clifueluser/mailsend
RUN ./configure --with-openssl=/usr
RUN make clean
RUN make
RUN make install
WORKDIR /home/clifueluser
RUN rm -Rfv ./mailsend
RUN git clone -b $GIT_BRANCH https://www.github.com/marcoM32/cliFuel.git
WORKDIR /home/clifueluser/cliFuel
RUN ./build-linux.sh

RUN chown -R clifueluser:clifueluser .

ENTRYPOINT ["/home/clifueluser/cliFuel/bin/linux/cliFuel-demon.sh"]