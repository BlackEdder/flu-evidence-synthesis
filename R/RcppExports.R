# This file was generated by Rcpp::compileAttributes
# Generator token: 10BE3573-1514-4C36-9D1C-5A225CD40393

#' MCMC based inference of the parameter values given the different data sets
#'
#' @param age_sizes A vector with the population size by each age {1,2,..}
#' @param ili The number of Influenza-like illness cases per week
#' @param mon_pop The number of people monitored for ili
#' @param n_pos The number of positive samples for the given strain (per week)
#' @param n_samples The total number of samples tested 
#' @param vaccine_calendar A vaccine calendar valid for that year
#' @param polymod_data Contact data for different age groups
#' @param init_sample The initial parameters needed to run the ODE model (typically one of the posterior sample created when running the inference)
#' @param mcmc_chain_length The number of MCMC steps to sample from
#' @param thinning Keep every so many samples
#' @param burn_in The number of initial samples to skip
#' @return A vector with posterior samples of the parameters (length of \code{mcmc_chain_length}/\code{thinning})
#'
#'
inference <- function(age_sizes, ili, mon_pop, n_pos, n_samples, vaccine_calendar, polymod_data, init_sample, mcmc_chain_length = 100000L, burn_in = 10000L, thinning = 100L) {
    .Call('fluEvidenceSynthesis_inference', PACKAGE = 'fluEvidenceSynthesis', age_sizes, ili, mon_pop, n_pos, n_samples, vaccine_calendar, polymod_data, init_sample, mcmc_chain_length, burn_in, thinning)
}

#' Update means when a new posterior sample is calculated
#'
#' @param means the current means of the parameters
#' @param v the new parameter values
#' @param n The number of posterior (mcmc) samples taken till now
#' @return The updated means given the new parameter sample
#'
.updateMeans <- function(means, v, n) {
    .Call('fluEvidenceSynthesis_updateMeans', PACKAGE = 'fluEvidenceSynthesis', means, v, n)
}

#' Update covariance matrix of posterior parameters
#'
#' Used to enable faster mixing of the mcmc chain
#' @param cov The current covariance matrix
#' @param v the new parameter values
#' @param means the current means of the parameters
#' @param n The number of posterior (mcmc) samples taken till now
#' @return The updated covariance matrix given the new parameter sample
#'
.updateCovariance <- function(cov, v, means, n) {
    .Call('fluEvidenceSynthesis_updateCovariance', PACKAGE = 'fluEvidenceSynthesis', cov, v, means, n)
}

#' Convert given week in given year into an exact date corresponding to the Monday of that week
#'
#' @param week The number of the week we need the date of
#' @param year The year
#' @return The date of the Monday in that week 
#'
getTimeFromWeekYear <- function(week, year) {
    .Call('fluEvidenceSynthesis_getTimeFromWeekYear', PACKAGE = 'fluEvidenceSynthesis', week, year)
}

#' Run the SEIR model for the given parameters
#'
#' @param age_sizes A vector with the population size by each age {1,2,..}
#' @param vaccine_calendar A vaccine calendar valid for that year
#' @param polymod_data Contact data for different age groups
#' @param susceptibility Vector with susceptibilities of each age group
#' @param transmissibility The transmissibility of the strain
#' @param init_pop The (log of) initial infected population
#' @param infection_delays Vector with the time of latent infection and time infectious
#' @param interval Interval (in days) between data points
#' @return A data frame with number of new cases after each interval during the year
#'
infection.model <- function(age_sizes, vaccine_calendar, polymod_data, susceptibility, transmissibility, init_pop, infection_delays, interval = 1L) {
    .Call('fluEvidenceSynthesis_runSEIRModel', PACKAGE = 'fluEvidenceSynthesis', age_sizes, vaccine_calendar, polymod_data, susceptibility, transmissibility, init_pop, infection_delays, interval)
}

#' Returns log likelihood of the predicted number of cases given the data for that week
#'
#' The model results in a prediction for the number of new cases in a certain age group and for a certain week. This function calculates the likelihood of that given the data on reported Influenza Like Illnesses and confirmed samples.
#'
#' @param epsilon Parameter for the probability distribution
#' @param psi Parameter for the probability distribution
#' @param predicted Number of cases predicted by your model
#' @param population_size The total population size in the relevant age group
#' @param ili_cases The number of Influenza Like Illness cases
#' @param ili_monitored The size of the population monitored for ILI
#' @param confirmed_positive The number of samples positive for the Influenza strain
#' @param confirmed_samples Number of samples tested for the Influenza strain
#'
llikelihood.cases <- function(epsilon, psi, predicted, population_size, ili_cases, ili_monitored, confirmed_positive, confirmed_samples) {
    .Call('fluEvidenceSynthesis_log_likelihood', PACKAGE = 'fluEvidenceSynthesis', epsilon, psi, predicted, population_size, ili_cases, ili_monitored, confirmed_positive, confirmed_samples)
}

#' Run an ODE model with the runge-kutta solver for testing purposes
#'
#' @param step_size The size of the step between returned time points
#' @param h_step The starting integration delta size
#'
.runRKF <- function(step_size = 0.1, h_step = 0.01) {
    .Call('fluEvidenceSynthesis_runPredatorPrey', PACKAGE = 'fluEvidenceSynthesis', step_size, h_step)
}

#' Run an ODE model with the simple step wise solver for testing purposes
#'
#' @param step_size The size of the step between returned time points
#' @param h_step The starting integration delta size
#'
.runStep <- function(step_size = 0.1, h_step = 1e-5) {
    .Call('fluEvidenceSynthesis_runPredatorPreySimple', PACKAGE = 'fluEvidenceSynthesis', step_size, h_step)
}

#' Adaptive MCMC algorithm implemented in C++
#'
#' MCMC which adapts its proposal distribution for faster convergence following:
#' Sherlock, C., Fearnhead, P. and Roberts, G.O. The Random Walk Metrolopois: Linking Theory and Practice Through a Case Study. Statistical Science 25, no.2 (2010): 172-190.
#'
#' @param lprior A function returning the log prior probability of the parameters 
#' @param llikelihood A function returning the log likelihood of the parameters given the data
#' @param nburn Number of iterations of burn in
#' @param initial Vector with starting parameter values
#' @param nbatch Number of batches to run (number of samples to return)
#' @param blen Length of each batch
#' 
#' @return Returns a list with the accepted samples and the corresponding llikelihood values
#'
#' @seealso \code{\link{adaptive.mcmc}} For a more flexible R frontend to this function.
#'
adaptive.mcmc.cpp <- function(lprior, llikelihood, nburn, initial, nbatch, blen = 1L) {
    .Call('fluEvidenceSynthesis_adaptiveMCMCR', PACKAGE = 'fluEvidenceSynthesis', lprior, llikelihood, nburn, initial, nbatch, blen)
}

#' Calculate number of influenza cases given a vaccination strategy
#'
#' @param age_sizes A vector with the population size by each age {1,2,..}
#' @param vaccine_calendar A vaccine calendar valid for that year
#' @param polymod_data Contact data for different age groups
#' @param sample The parameters needed to run the ODE model (typically one of the posterior sample created when running the inference)
#' @return A data frame with the total number of influenza cases in that year
#'
vaccinationScenario <- function(age_sizes, vaccine_calendar, polymod_data, sample) {
    .Call('fluEvidenceSynthesis_vaccinationScenario', PACKAGE = 'fluEvidenceSynthesis', age_sizes, vaccine_calendar, polymod_data, sample)
}
