class CreateStations < ActiveRecord::Migration
  def change
    create_table :stations do |t|
      t.string :name
      t.decimal :lat
      t.decimal :lon
      t.string :notes

      t.timestamps null: false
    end
  end
end
