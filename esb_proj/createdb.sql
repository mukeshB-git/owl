


SET @OLD_UNIQUE_CHECKS=@@UNIQUE_CHECKS, UNIQUE_CHECKS=0;
SET @OLD_FOREIGN_KEY_CHECKS=@@FOREIGN_KEY_CHECKS, FOREIGN_KEY_CHECKS=0;
SET @OLD_SQL_MODE=@@SQL_MODE, SQL_MODE='TRADITIONAL,ALLOW_INVALID_DATES';

-- -----------------------------------------------------
-- Schema esb
-- -----------------------------------------------------
DROP SCHEMA IF EXISTS `esb` ;

-- -----------------------------------------------------
-- Schema esb
-- -----------------------------------------------------
CREATE SCHEMA IF NOT EXISTS `esb` DEFAULT CHARACTER SET utf8 ;
USE `esb` ;

-- -----------------------------------------------------
-- Table `esb`.`esb_request`
-- -----------------------------------------------------
DROP TABLE IF EXISTS `esb`.`esb_request` ;

CREATE TABLE IF NOT EXISTS `esb`.`esb_request` (
  `id` INT NOT NULL AUTO_INCREMENT,
  `sender_id` VARCHAR(45) NOT NULL,
  `dest_id` VARCHAR(45) NOT NULL,
  `message_type` VARCHAR(45) NOT NULL,
  `reference_id` VARCHAR(45) NOT NULL,
  `message_id` VARCHAR(45) NOT NULL COMMENT 'A unique ID for the message instance.',
  `received_on` DATETIME NOT NULL,
  `data_location` TEXT NULL,
  `status` VARCHAR(20) NOT NULL,
  `status_details` TEXT NULL,
  PRIMARY KEY (`id`)
  )
ENGINE = InnoDB;


-- -----------------------------------------------------
-- Table `esb`.`routes`
-- -----------------------------------------------------
DROP TABLE IF EXISTS `esb`.`routes` ;

CREATE TABLE IF NOT EXISTS `esb`.`routes` (
  `route_id` INT NOT NULL AUTO_INCREMENT,
  `sender` VARCHAR(45) NOT NULL,
  `destination` VARCHAR(45) NOT NULL,
  `message_type` VARCHAR(45) NOT NULL,
  `is_active` BIT(1) NOT NULL,
  PRIMARY KEY (`route_id`),
  UNIQUE INDEX `UK_sender_dest_msg_type` (`message_type` ASC, `destination` ASC, `sender` ASC))
ENGINE = InnoDB;


-- -----------------------------------------------------
-- Table `esb`.`transform_config`
-- -----------------------------------------------------
DROP TABLE IF EXISTS `esb`.`transform_config` ;

CREATE TABLE IF NOT EXISTS `esb`.`transform_config` (
  `id` INT NOT NULL AUTO_INCREMENT,
  `route_id` INT NOT NULL,
  `config_key` VARCHAR(45) NOT NULL,
  `config_value` TEXT NULL,
  PRIMARY KEY (`id`),
  INDEX `route_idx` (`route_id` ASC),
  UNIQUE INDEX `UK_route_config` (`route_id` ASC, `config_key` ASC),
  CONSTRAINT `route`
    FOREIGN KEY (`route_id`)
    REFERENCES `esb`.`routes` (`route_id`))
   
ENGINE = InnoDB;


-- -----------------------------------------------------
-- Table `esb`.`transport_config`
-- -----------------------------------------------------
DROP TABLE IF EXISTS `esb`.`transport_config` ;

CREATE TABLE IF NOT EXISTS `esb`.`transport_config` (
  `id` INT NOT NULL AUTO_INCREMENT,
  `route_id` INT NULL,
  `config_key` VARCHAR(45) NULL,
  `config_value` TEXT NULL,
  PRIMARY KEY (`id`),
  INDEX `route_idx` (`route_id` ASC),
  UNIQUE INDEX `UK_route_config` (`config_key` ASC, `route_id` ASC),
 
    FOREIGN KEY (`route_id`)
    REFERENCES `esb`.`routes` (`route_id`))
   
ENGINE = InnoDB;


SET SQL_MODE=@OLD_SQL_MODE;
SET FOREIGN_KEY_CHECKS=@OLD_FOREIGN_KEY_CHECKS;
SET UNIQUE_CHECKS=@OLD_UNIQUE_CHECKS;
