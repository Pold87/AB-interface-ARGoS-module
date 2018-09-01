source("myplothelpers.R")

## April 2018
## Volker Strobel
## This script creates the plots for the ANTS 2018 paper

## First mount the data folder of the experiments to data.base
## bash ~/Documents/mountscripts/mountestimation.command

do.exp1 <- FALSE
do.exp2 <- FALSE
do.exp3 <- FALSE

difficulties <- seq(34, 48, 2)
thresholds <- seq(30 * 10^4, 6 * 10 ^ 4, - 2 * 10^4)
num.byz <- 0:12
style <- "blockchain"
nodes <- c(0, 6)

dates.exp1 <- c("20-04-2018")
dates.exp2 <- c("14-03-2018", "17-04-2018", "18-04-2018")
dates.exp3.nonsecure <- c("15-04-2018", "24-04-2018") ## non-secure, Byz always 1.0
dates.exp3.secure <- c("16-04-2018") ## secure, Byz always 1.0

MULTIPLIER <- 10^7 # Multiplier for converting int to float

data.base <- "~/localestimation/" ## Folder containing the results of the experiments
report.dir <- "dummy/" ## Folder images for the paper
N = 20

## Experiment 1 (Increasing difficulty)
## Old versions (experiments from March)
create.df.old.exp1 <- function(max.trials=50) {
    df <- data.frame()
    for (dat in dates.exp1) {
        for (i in 1:max.trials) {
            for (d in difficulties) {
                for (node in nodes){

                    trials.name <- sprintf("%s/experiment1-node%d-%s/num20_black%d_byz0_run%d-blockchain.RUN1", data.base, node, dat, d, i)

                    if (file.exists(trials.name)) {
                        X <- tryCatch(read.table(trials.name, header=T), error=function(e) NULL)
                        if (nrow(X) != 0 && !is.null(X)){

                            ## extract last row
                            X <- X[nrow(X), ]                            
                            X$difficulty = round(d / (100 - d), 2)
                            X$actual = d / 100
                            X$predicted = 1 - X$mean / MULTIPLIER
                            if (nrow(df) == 0) {
                                df <- X
                            } else  {
                                df <- rbind(df, X)
                            }
                        }
                    }           
                }
            }
        }
    }
    return(df)    
}


## New verion (experiments from April, 20)
create.df.exp1 <- function(max.trials=50) {
    t <- 140000
    df <- data.frame()
    for (dat in dates.exp1) {
        for (i in 1:max.trials) {
            for (d in difficulties) {
                for (node in nodes){

                    trials.name <- sprintf("%s/experiment1-node%d-%s/%d/num20_black%d_byz0_run%d-blockchain.RUN1", data.base, node, dat, t, d, i)

                    if (file.exists(trials.name)) {
                        X <- tryCatch(read.table(trials.name, header=T), error=function(e) NULL)
                        if (nrow(X) != 0 && !is.null(X)){

                            ## extract last row
                            X <- X[nrow(X), ]                            
                            X$difficulty = round(d / (100 - d), 2)
                            X$actual = d / 100
                            X$predicted = 1 - X$mean / MULTIPLIER
                            if (nrow(df) == 0) {
                                df <- X
                            } else  {
                                df <- rbind(df, X)
                            }
                        }
                    }           
                }
            }
        }
    }
    return(df)    
}


## Experiment 2 (Varying threshold)
create.df.exp2 <- function(max.trials=30) {
    d <- 40
    df <- data.frame()
    for (dat in dates.exp2) {
        for (i in 1:max.trials) {
            for (t in thresholds) {
                for (node in nodes){

                    trials.name <- sprintf("%s/experiment2-node%d-%s/%d/num20_black%d_byz0_run%d-blockchain.RUN1", data.base, node, dat, t, d, i)
                    if (file.exists(trials.name)) {
                        X <- tryCatch(read.table(trials.name, header=T), error=function(e) NULL)
                        if (nrow(X) != 0 && !is.null(X)){

                            ## extract last row
                            X <- X[nrow(X), ]

                            X$threshold <- t / 10^7
                            X$difficulty = round(d / (100 - d), 2)
                            X$actual = d / 100
                            X$predicted = 1 - X$mean / MULTIPLIER
                            if (nrow(df) == 0) {
                                df <- X
                            } else  {
                                df <- rbind(df, X)
                            }
                        }
                    }           
                }
            }
        }
    }
    return(df)    
}


## Experiment 3 (Increasing the number of Byzantine robots)
create.df.exp3 <- function(dates, max.trials=30, secure=T) {
    d <- 40
    df <- data.frame()
    t <- 140000
    for (dat in dates) {
        for (i in 1:max.trials) {
            for (b in num.byz) {
                for (node in nodes){

                    trials.name <- sprintf("%s/experiment3-secure%d-vote-node%d-%s/%d/num20_black%d_byz%d_run%d-blockchain.RUN1", data.base, secure, node, dat, t, d, b, i)
                    if (file.exists(trials.name)) {
                        X <- tryCatch(read.table(trials.name, header=T), error=function(e) NULL)
                        if (nrow(X) != 0 && !is.null(X)){

                            ## extract last row
                            X <- X[nrow(X), ]
                            X$byz <- b
                            X$threshold <- t / 10^7
                            X$difficulty = round(d / (100 - d), 2)
                            X$actual = d / 100
                            X$predicted = 1 - X$mean / MULTIPLIER
                            if (nrow(df) == 0) {
                                df <- X
                            } else  {
                                df <- rbind(df, X)
                            }
                        }
                    }
                }
            }
        }
    }
    return(df)    
}


add.stats <- function(.df) {

    .df$error <- .df$actual - .df$predicted
    .df$absError <- abs(.df$error)
    .df$squaredError <- .df$error ^ 2
    .df$consWhite <- .df$predicted < 0.5
    
    return(.df)
}


###############
## Experiment 1
###############
if (do.exp1) {
    print("Reading data of Experiment 1")
    df <- create.df.exp1() ## Iterate over runs and create big df
    df <- df[df$clock < 1000, ] ## Remove outliers
    df <- add.stats(df)

    print(head(df))

    ## Save as CSV
    f1 <- "experiment1_ants2018.csv"
    print(paste("Writing to file", f1))
    write.csv(df, f1, row.names = FALSE, quote=FALSE)

    print("Creating plots for Experiment 1")

    source("myplothelpers.R")
    plot.error.gg(df,
                  xlab=expression(rho['b']),
                  ylab=expression(bar(x)[symbol("\052")*","*infinity]),
                  sprintf("exp1_error.pdf"),
                  report.dir)  

    source("myplothelpers.R")
    plot.cons.gg(df,
                 xlab=expression(rho['b']),
                 ylab=expression(T[N]),
                 sprintf("constime.pdf"),
                 report.dir)  


    source("myplothelpers.R")
    plot.exit.prob.gg2(df,
                       xlab=expression(rho['b']),
                       ylab=expression(E[N]),
                       sprintf("exitprob.pdf"),
                       report.dir)  


    source("myplothelpers.R")
    plot.abs.error.gg(df,
                      xlab=expression(rho['b']),
                      ylab=expression(MAE),
                      sprintf("absMeanError.pdf"),
                      report.dir)

    source("myplothelpers.R")
    plot.MAE.by.rho.gg(df,
                       xlab=expression(rho['b']),
                       ylab=expression("MAE"),
                       sprintf("MAE_rho.pdf"),
                       report.dir)      


    source("myplothelpers.R")
    plot.blockchain.size.gg(df,
                            xlab=expression(rho['b']),
                            ylab=expression(BC[MB]),
                            sprintf("blockchain_size.pdf"),
                            report.dir)  

    
}

###############
## Experiment 2
###############
if (do.exp2) {
    print("Reading data of Experiment 2")
    df2 <- create.df.exp2() ## Iterate over runs and create big df
    df2 <- add.stats(df2)

    print(head(df2))
    f2 <- "experiment2_ants2018.csv"
    print(paste("Writing to file", f2))
    write.csv(df2, f2, row.names = FALSE, quote=FALSE)
    
    ## Experiment 2
    print("Creating plots for experiment 2")

    source("myplothelpers.R")
    plot.blockchain.size.by.tau.gg(df2,
                                   xlab=expression(tau),
                                   ylab=expression(BC[MB]),
                                   sprintf("blockchain_size_tau.pdf"),
                                   report.dir)  



    plot.error.by.tau.gg(df2,
                         xlab=expression(tau),
                         ylab=expression("Error"),
                         sprintf("error_tau.pdf"),
                         report.dir)  

    source("myplothelpers.R")
    plot.MAE.by.tau.gg(df2,
                       xlab=expression(tau),
                       ylab="MAE",
                       sprintf("MAE_tau.pdf"),
                       report.dir)  


    plot.MSE.by.tau.gg(df2,
                       xlab=expression(tau),
                       ylab=expression("MSE"),
                       sprintf("MSE_tau.pdf"),
                       report.dir)  

    source("myplothelpers.R")
    plot.cons.by.tau.gg(df2,
                        xlab=expression(tau),
                        ylab=expression(T[N]),
                        sprintf("exp2-constime.pdf"),
                        report.dir)  


}

###############
## Experiment 3
###############
if (do.exp3) {

    print("Reading data of Experiment 3")
    df3.secure <- create.df.exp3(dates=dates.exp3.secure, secure=T) ## Iterate over runs and create big df

    df3.secure <- df3.secure[df3.secure$byz < 10, ]
    df3.secure <- add.stats(df3.secure)
    print(head(df3.secure))
    

    df3.nonsecure <- create.df.exp3(dates=dates.exp3.nonsecure, secure=F) ## Iterate over runs and create big df
    df3.nonsecure <- df3.nonsecure[df3.nonsecure$byz < 10, ] ## More than 9 is not interesting
    df3.nonsecure <- add.stats(df3.nonsecure)


    ## Save as CSV
    f3.secure <- "experiment3_secure_ants2018.csv"
    print(paste("Writing to file", f3.secure))
    write.csv(df3.secure, f3.secure, row.names = FALSE, quote=FALSE)

    
    f3.nonsecure <- "experiment3_nonsecure_ants2018.csv"
    print(paste("Writing to file", f3.nonsecure))
    write.csv(df3.nonsecure, f3.nonsecure, row.names = FALSE, quote=FALSE)


    source("myplothelpers.R")
    ## Experiment 3 non-secure
    print("Creating plots for experiment 3 (non-secure)")
    plot.MAE.by.byz.gg(df3.nonsecure,
                       xlab=expression(italic(k)),
                       ylab="MAE",
                       sprintf("MAE_byz-nonsecure.pdf"),
                       report.dir) 
       
    
    ## Experiment 3 secure
    print("Creating plots for experiment 3 (secure)")
    plot.MAE.by.byz.gg(df3.secure,
                       xlab=expression(italic(k)),
                       ylab="MAE",
                       sprintf("MAE_byz-secure.pdf"),
                       report.dir) 
    

}
